#include "tdclient.h"

#include <toml++/toml.h>

#include <cstring>
#include <dylib.hpp>
#include <format>
#include <print>

TdConfig ReadConfig(std::string_view filename) {
    auto config = toml::parse_file(filename);
    std::string mode = config["default"]["mode"].value_or("openctp");
    std::string platform = config["default"]["platform"].value_or("lin64");
    std::string front_td = config[mode]["front_td"].value_or("");
    std::string auth_id = config[mode]["auth_id"].value_or("");
    std::string auth_code = config[mode]["auth_code"].value_or("");
    std::string broker_id = config[mode]["broker_id"].value_or("");
    std::string user_id = config[mode]["user_id"].value_or("");
    std::string password = config[mode]["password"].value_or("");
    return TdConfig{
        mode,
        platform,
        front_td,
        auth_id,
        auth_code,
        broker_id,
        user_id,
        password,
    };
}

void TdClient::Start() {
    // read config
    _config = ReadConfig("config.toml");

    // load dylib
    auto dylib_path = std::format("{}/{}", _config.mode, _config.platform);
    _lib_td.emplace(dylib_path, "thosttraderapi_se.so", dylib::no_filename_decorations);
    auto GetApiVersion = _lib_td->get_function<const char *()>("_ZN19CThostFtdcTraderApi13GetApiVersionEv");
    std::println("td_ver={}", GetApiVersion());
    auto CreateFtdcTraderApi = _lib_td->get_function<CThostFtdcTraderApi *(const char *)>("_ZN19CThostFtdcTraderApi19CreateFtdcTraderApiEPKc");

    // register
    _tdapi = CreateFtdcTraderApi("");
    _tdapi->RegisterSpi(this);
    _tdapi->RegisterFront(_config.front_td.data());

    // connect
    _tdapi->Init();
    _sem.acquire();

    // auth
    CThostFtdcReqAuthenticateField auth_req{};
    _config.broker_id.copy(auth_req.BrokerID, _config.broker_id.length());
    _config.user_id.copy(auth_req.UserID, _config.user_id.length());
    _config.auth_id.copy(auth_req.AppID, _config.auth_id.length());
    _config.auth_code.copy(auth_req.AuthCode, _config.auth_code.length());
    _tdapi->ReqAuthenticate(&auth_req, ++_reqId);
    _sem.acquire();

    // login
    CThostFtdcReqUserLoginField login_req{};
    _config.broker_id.copy(login_req.BrokerID, _config.broker_id.length());
    _config.user_id.copy(login_req.UserID, _config.user_id.length());
    _config.password.copy(login_req.Password, _config.password.length());
    _tdapi->ReqUserLogin(&login_req, ++_reqId);
    _sem.acquire();
}

void TdClient::OnFrontConnected() {
    std::println("OnFrontConnected");
    _sem.release();
}

void TdClient::OnFrontDisconnected(int nReason) {
    std::println("OnFrontDisconnected, reason={}", nReason);
}

void TdClient::OnHeartBeatWarning(int nTimeLapse) {
    std::println("OnHeartBeatWarning, time={}", nTimeLapse);
}

void TdClient::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::println("OnRspAuthenticate");
    if (bIsLast) {
        _sem.release();
    }
}

void TdClient::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::println("OnRspUserLogin");
    std::println("{} login at {} {}", pRspUserLogin->UserID, pRspUserLogin->TradingDay, pRspUserLogin->LoginTime);
    if (bIsLast) {
        _sem.release();
    }
}

void TdClient::ReqSettlementInfo() {
    CThostFtdcSettlementInfoConfirmField req{};
    _config.broker_id.copy(req.BrokerID, _config.broker_id.length());
    _config.user_id.copy(req.InvestorID, _config.user_id.length());
    _tdapi->ReqSettlementInfoConfirm(&req, ++_reqId);
    _sem.acquire();
}

void TdClient::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::println("OnRspSettlementInfoConfirm");
    std::println("brokder_id={}, investor_id={}, confirm_date={}, confirm_time={}, settle_id={}, account_id={}, currency_id={}",
                 pSettlementInfoConfirm->BrokerID,
                 pSettlementInfoConfirm->InvestorID,
                 pSettlementInfoConfirm->ConfirmDate,
                 pSettlementInfoConfirm->ConfirmTime,
                 pSettlementInfoConfirm->SettlementID,
                 pSettlementInfoConfirm->AccountID,
                 pSettlementInfoConfirm->CurrencyID);
    if (bIsLast) {
        _sem.release();
    }
}

void TdClient::QryInvestorPosition() {
    CThostFtdcQryInvestorPositionField req{};
    _tdapi->ReqQryInvestorPosition(&req, ++_reqId);
    _sem.acquire();
}

void TdClient::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::println("OnRspQryInvestorPosition");

    if (pInvestorPosition) {
        std::println("symbol={},position={}", pInvestorPosition->InstrumentID, pInvestorPosition->Position);
    } else {
        std::println("hold nothing");
    }

    if (bIsLast) {
        _sem.release();
    }
}

void TdClient::QryTradingAccount() {
    CThostFtdcQryTradingAccountField req{};
    _config.broker_id.copy(req.BrokerID, _config.broker_id.length());
    _config.user_id.copy(req.InvestorID, _config.user_id.length());
    auto ret = _tdapi->ReqQryTradingAccount(&req, ++_reqId);
    _sem.acquire();
}

void TdClient::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::println("OnRspQryTradingAccount");
    std::println("available={}", pTradingAccount->Available);
    if (bIsLast) {
        _sem.release();
    }
}