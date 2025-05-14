#include "tdclient.h"

#include <cstring>
#include <dylib.hpp>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "../utils.hpp"
#include "ThostFtdcUserApiDataType.h"

struct TdClient::Impl {
    CtpConfig cfg{};
    std::optional<dylib> lib{};
};

TdClient::TdClient(std::string_view cfg_file) : pImpl(std::make_unique<Impl>()) {
    // read toml config
    pImpl->cfg = read_config(cfg_file);
}

TdClient::~TdClient() { _tdapi->Release(); }

void TdClient::Start() {
    // load dylib
    auto dylib_path = std::format("{}/{}", pImpl->cfg.lib_dir, pImpl->cfg.platform);
    pImpl->lib.emplace(dylib_path, "thosttraderapi_se.so", dylib::no_filename_decorations);

    auto GetApiVersion = pImpl->lib->get_function<const char *()>("_ZN19CThostFtdcTraderApi13GetApiVersionEv");
    std::println("td_ver={}", GetApiVersion());
    auto CreateFtdcTraderApi = pImpl->lib->get_function<CThostFtdcTraderApi *(const char *)>("_ZN19CThostFtdcTraderApi19CreateFtdcTraderApiEPKc");

    // register
    _tdapi = CreateFtdcTraderApi("");
    _tdapi->RegisterSpi(this);
    _tdapi->RegisterFront(pImpl->cfg.front_td.data());

    // connect
    _tdapi->Init();
    _sem.acquire();

    // auth
    CThostFtdcReqAuthenticateField auth_req{};
    pImpl->cfg.broker_id.copy(auth_req.BrokerID, pImpl->cfg.broker_id.length());
    pImpl->cfg.user_id.copy(auth_req.UserID, pImpl->cfg.user_id.length());
    pImpl->cfg.auth_id.copy(auth_req.AppID, pImpl->cfg.auth_id.length());
    pImpl->cfg.auth_code.copy(auth_req.AuthCode, pImpl->cfg.auth_code.length());
    _tdapi->ReqAuthenticate(&auth_req, ++_reqId);
    _sem.acquire();

    // login
    CThostFtdcReqUserLoginField login_req{};
    pImpl->cfg.broker_id.copy(login_req.BrokerID, pImpl->cfg.broker_id.length());
    pImpl->cfg.user_id.copy(login_req.UserID, pImpl->cfg.user_id.length());
    pImpl->cfg.password.copy(login_req.Password, pImpl->cfg.password.length());
    _tdapi->ReqUserLogin(&login_req, ++_reqId);
    _sem.acquire();
}

void TdClient::OnFrontConnected() {
    std::println("OnFrontConnected");
    _sem.release();
}

void TdClient::OnFrontDisconnected(int nReason) {
    std::println("OnFrontDisconnected: {}", errconfig::discon_errors.at(nReason));
}

void TdClient::OnHeartBeatWarning(int nTimeLapse) {
    std::println("OnHeartBeatWarning, time={}", nTimeLapse);
}

void TdClient::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pRspAuthenticateField, pRspInfo);

    if (bIsLast) _sem.release();
}

void TdClient::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pRspUserLogin, pRspInfo);

    if (bIsLast) _sem.release();
}

void TdClient::SettlementInfo() {
    CThostFtdcSettlementInfoConfirmField req{};
    _tdapi->ReqSettlementInfoConfirm(&req, ++_reqId);
    _sem.acquire();
}

void TdClient::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pSettlementInfoConfirm, pRspInfo);

    if (bIsLast) _sem.release();
}

void TdClient::QryInvestorPosition() {
    CThostFtdcQryInvestorPositionField req{};
    _tdapi->ReqQryInvestorPosition(&req, ++_reqId);
    _sem.acquire();
}

void TdClient::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pInvestorPosition, pRspInfo);

    if (bIsLast) _sem.release();
}

void TdClient::QryTradingAccount() {
    CThostFtdcQryTradingAccountField req{};
    _tdapi->ReqQryTradingAccount(&req, ++_reqId);
    _sem.acquire();
}

void TdClient::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pTradingAccount, pRspInfo);

    if (bIsLast) _sem.release();
}

// insert order
void TdClient::OrderInsert(std::string_view symbol, TThostFtdcDirectionType direction, TThostFtdcOffsetFlagType offset, TThostFtdcPriceType price, TThostFtdcVolumeType lot, bool is_stop = false) {
    CThostFtdcInputOrderField req{};
    req.Direction = direction;
    req.CombOffsetFlag[0] = offset;
    req.VolumeTotalOriginal = lot;
    if (is_stop) {
        req.StopPrice = price;
    } else {
        req.LimitPrice = price;
    }
    // todo
    _tdapi->ReqOrderInsert(&req, ++_reqId);
}

void TdClient::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pInputOrder, pRspInfo);

    if (bIsLast) _sem.release();
}

void TdClient::OnRtnOrder(CThostFtdcOrderField *pOrder) {
    print_struct(pOrder);
}

void TdClient::OnRtnTrade(CThostFtdcTradeField *pTrade) {
    print_struct(pTrade);
}

bool TdClient::Buy(std::string_view symbol, int lot, double price) {
    // todo
    this->OrderInsert(symbol, THOST_FTDC_D_Buy, THOST_FTDC_OF_Open, price, lot);
}

bool TdClient::Sell(std::string_view symbol, int lot, double price) {
    // todo
    this->OrderInsert(symbol, THOST_FTDC_D_Sell, THOST_FTDC_OF_Close, price, lot);
}

bool TdClient::SellShort(std::string_view symbol, int lot, double price) {
    // todo
    this->OrderInsert(symbol, THOST_FTDC_D_Sell, THOST_FTDC_OF_Open, price, lot);
}

bool TdClient::Buy2Cover(std::string_view symbol, int lot, double price) {
    // todo
    this->OrderInsert(symbol, THOST_FTDC_D_Buy, THOST_FTDC_OF_Close, price, lot);
}

// cancel order
void TdClient::OrderAction() {
    CThostFtdcInputOrderActionField req{};
    // todo
    _tdapi->ReqOrderAction(&req, ++_reqId);
}

void TdClient::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pInputOrderAction, pRspInfo);

    if (bIsLast) _sem.release();
}

void TdClient::QryInstrument(std::vector<std::string> exchange_ids) {
    if (exchange_ids.empty()) {
        CThostFtdcQryInstrumentField req{};
        // 获取所有交易所全部合约列表
        _tdapi->ReqQryInstrument(&req, ++_reqId);
    } else {
        for (auto &&e : exchange_ids) {
            CThostFtdcQryInstrumentField req{};
            e.copy(req.ExchangeID, e.length());
            // 获取对应交易所全部合约列表
            _tdapi->ReqQryInstrument(&req, ++_reqId);
        }
    }
}

void TdClient::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pInstrument, pRspInfo);

    if (bIsLast) _sem.release();
}

void TdClient::QryExchange() {
    CThostFtdcQryExchangeField req{};
    _tdapi->ReqQryExchange(&req, ++_reqId);
}

void TdClient::OnRspQryExchange(CThostFtdcExchangeField *pExchange, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pExchange, pRspInfo);

    if (bIsLast) _sem.release();
}

void TdClient::QryProduct() {
    CThostFtdcQryProductField req{};
    _tdapi->ReqQryProduct(&req, ++_reqId);
}

void TdClient::OnRspQryProduct(CThostFtdcProductField *pProduct, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pProduct, pRspInfo);

    if (bIsLast) _sem.release();
}
