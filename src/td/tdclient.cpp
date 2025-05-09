#include "tdclient.h"

#include <toml++/toml.h>

#include <cstring>
#include <dylib.hpp>
#include <format>
#include <print>
#include <utils/parser.hpp>
#include <utils/repr.hpp>

#include "../config.hpp"

template <typename T>
void handle_resp(T const *ptr, CThostFtdcRspInfoField const *pRspInfo) noexcept {
    auto name = ylt::reflection::get_struct_name<T>();
    if (pRspInfo) {
        std::println("{}, {}", errconfig::get(pRspInfo->ErrorID), name);
    }

    if (ptr) {
        print_struct(ptr);
    } else {
        std::println("nothing in {}", name);
    }
}

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
    // ctpconfig::cfg = ReadConfig("config.toml");

    // load dylib
    auto dylib_path = std::format("{}/{}", ctpconfig::cfg.mode, ctpconfig::cfg.platform);
    _lib_td.emplace(dylib_path, "thosttraderapi_se.so", dylib::no_filename_decorations);
    auto GetApiVersion = _lib_td->get_function<const char *()>("_ZN19CThostFtdcTraderApi13GetApiVersionEv");
    std::println("td_ver={}", GetApiVersion());
    auto CreateFtdcTraderApi = _lib_td->get_function<CThostFtdcTraderApi *(const char *)>("_ZN19CThostFtdcTraderApi19CreateFtdcTraderApiEPKc");

    // register
    _tdapi = CreateFtdcTraderApi("");
    _tdapi->RegisterSpi(this);
    _tdapi->RegisterFront(const_cast<char *>(ctpconfig::cfg.front_td.data()));

    // connect
    _tdapi->Init();
    _sem.acquire();

    // auth
    CThostFtdcReqAuthenticateField auth_req{};
    ctpconfig::cfg.broker_id.copy(auth_req.BrokerID, ctpconfig::cfg.broker_id.length());
    ctpconfig::cfg.user_id.copy(auth_req.UserID, ctpconfig::cfg.user_id.length());
    ctpconfig::cfg.auth_id.copy(auth_req.AppID, ctpconfig::cfg.auth_id.length());
    ctpconfig::cfg.auth_code.copy(auth_req.AuthCode, ctpconfig::cfg.auth_code.length());
    _tdapi->ReqAuthenticate(&auth_req, ++_reqId);
    _sem.acquire();

    // login
    CThostFtdcReqUserLoginField login_req{};
    ctpconfig::cfg.broker_id.copy(login_req.BrokerID, ctpconfig::cfg.broker_id.length());
    ctpconfig::cfg.user_id.copy(login_req.UserID, ctpconfig::cfg.user_id.length());
    ctpconfig::cfg.password.copy(login_req.Password, ctpconfig::cfg.password.length());
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
    handle_resp(pRspAuthenticateField, pRspInfo);

    if (bIsLast) {
        _sem.release();
    }
}

void TdClient::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pRspUserLogin, pRspInfo);

    if (bIsLast) {
        _sem.release();
    }
}

void TdClient::ReqSettlementInfo() {
    CThostFtdcSettlementInfoConfirmField req{};
    // ctpconfig::cfg.broker_id.copy(req.BrokerID, ctpconfig::cfg.broker_id.length());
    // ctpconfig::cfg.user_id.copy(req.InvestorID, ctpconfig::cfg.user_id.length());
    _tdapi->ReqSettlementInfoConfirm(&req, ++_reqId);
    _sem.acquire();
}

void TdClient::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pSettlementInfoConfirm, pRspInfo);

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
    handle_resp(pInvestorPosition, pRspInfo);

    if (bIsLast) {
        _sem.release();
    }
}

void TdClient::QryTradingAccount() {
    CThostFtdcQryTradingAccountField req{};
    // ctpconfig::cfg.broker_id.copy(req.BrokerID, ctpconfig::cfg.broker_id.length());
    // ctpconfig::cfg.user_id.copy(req.InvestorID, ctpconfig::cfg.user_id.length());
    auto ret = _tdapi->ReqQryTradingAccount(&req, ++_reqId);
    _sem.acquire();
}

void TdClient::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pTradingAccount, pRspInfo);

    if (bIsLast) {
        _sem.release();
    }
}