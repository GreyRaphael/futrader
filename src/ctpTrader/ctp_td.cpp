#include "ctp_td.h"

#include <config_parser.h>

#include <cstring>
#include <dylib.hpp>
#include <error_parser.hpp>
#include <format>
#include <memory>
#include <optional>
#include <print>
#include <string>
#include <string_view>

struct CtpTdClient::Impl {
    CtpConfig cfg{};
    std::optional<dylib> lib{};
};

CtpTdClient::CtpTdClient(std::string_view cfg_file) : pImpl(std::make_unique<Impl>()) {
    // read toml config
    pImpl->cfg = CtpConfig::read_config(cfg_file);
}

CtpTdClient::~CtpTdClient() { _tdapi->Release(); }

void CtpTdClient::Start() {
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

void CtpTdClient::OnFrontConnected() {
    std::println("OnFrontConnected");
    _sem.release();
}

void CtpTdClient::OnFrontDisconnected(int nReason) {
    std::println("OnFrontDisconnected: {}", errconfig::discon_errors.at(nReason));
}

void CtpTdClient::OnHeartBeatWarning(int nTimeLapse) {
    std::println("OnHeartBeatWarning, time={}", nTimeLapse);
}

void CtpTdClient::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pRspAuthenticateField, pRspInfo);

    if (bIsLast) _sem.release();
}

void CtpTdClient::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pRspUserLogin, pRspInfo);

    if (bIsLast) _sem.release();
}

void CtpTdClient::SettlementInfo() {
    CThostFtdcSettlementInfoConfirmField req{};
    _tdapi->ReqSettlementInfoConfirm(&req, ++_reqId);
    _sem.acquire();
}

void CtpTdClient::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pSettlementInfoConfirm, pRspInfo);

    if (bIsLast) _sem.release();
}

void CtpTdClient::QryInvestorPosition() {
    CThostFtdcQryInvestorPositionField req{};
    _tdapi->ReqQryInvestorPosition(&req, ++_reqId);
    _sem.acquire();
}

void CtpTdClient::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pInvestorPosition, pRspInfo);

    if (bIsLast) _sem.release();
}

void CtpTdClient::QryTradingAccount() {
    CThostFtdcQryTradingAccountField req{};
    _tdapi->ReqQryTradingAccount(&req, ++_reqId);
    _sem.acquire();
}

void CtpTdClient::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pTradingAccount, pRspInfo);

    if (bIsLast) _sem.release();
}

// insert order
void CtpTdClient::OrderInsert(std::string_view symbol, TThostFtdcDirectionType direction, TThostFtdcOffsetFlagType offset, TThostFtdcPriceType price, TThostFtdcVolumeType lot, bool is_stop = false) {
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

void CtpTdClient::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pInputOrder, pRspInfo);

    if (bIsLast) _sem.release();
}

void CtpTdClient::OnRtnOrder(CThostFtdcOrderField *pOrder) {
    print_struct(pOrder);
}

void CtpTdClient::OnRtnTrade(CThostFtdcTradeField *pTrade) {
    print_struct(pTrade);
}

bool CtpTdClient::Buy(std::string_view symbol, int lot, double price) {
    // todo
    this->OrderInsert(symbol, THOST_FTDC_D_Buy, THOST_FTDC_OF_Open, price, lot);
}

bool CtpTdClient::Sell(std::string_view symbol, int lot, double price) {
    // todo
    this->OrderInsert(symbol, THOST_FTDC_D_Sell, THOST_FTDC_OF_Close, price, lot);
}

bool CtpTdClient::SellShort(std::string_view symbol, int lot, double price) {
    // todo
    this->OrderInsert(symbol, THOST_FTDC_D_Sell, THOST_FTDC_OF_Open, price, lot);
}

bool CtpTdClient::Buy2Cover(std::string_view symbol, int lot, double price) {
    // todo
    this->OrderInsert(symbol, THOST_FTDC_D_Buy, THOST_FTDC_OF_Close, price, lot);
}

// cancel order
void CtpTdClient::OrderAction() {
    CThostFtdcInputOrderActionField req{};
    // todo
    _tdapi->ReqOrderAction(&req, ++_reqId);
}

void CtpTdClient::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pInputOrderAction, pRspInfo);

    if (bIsLast) _sem.release();
}

void CtpTdClient::QryInstrument(std::vector<std::string> exchange_ids) {
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

void CtpTdClient::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pInstrument, pRspInfo);

    if (bIsLast) _sem.release();
}

void CtpTdClient::QryExchange() {
    CThostFtdcQryExchangeField req{};
    _tdapi->ReqQryExchange(&req, ++_reqId);
}

void CtpTdClient::OnRspQryExchange(CThostFtdcExchangeField *pExchange, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pExchange, pRspInfo);

    if (bIsLast) _sem.release();
}

void CtpTdClient::QryProduct() {
    CThostFtdcQryProductField req{};
    _tdapi->ReqQryProduct(&req, ++_reqId);
}

void CtpTdClient::OnRspQryProduct(CThostFtdcProductField *pProduct, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pProduct, pRspInfo);

    if (bIsLast) _sem.release();
}

void CtpTdClient::QryInstrumentCommissionRate() {
    CThostFtdcQryInstrumentCommissionRateField req{};
    _tdapi->ReqQryInstrumentCommissionRate(&req, ++_reqId);
}

void CtpTdClient::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pInstrumentCommissionRate, pRspInfo);

    if (bIsLast) _sem.release();
}

void CtpTdClient::QryInstrumentOrderCommRate() {
    CThostFtdcQryInstrumentOrderCommRateField req{};
    _tdapi->ReqQryInstrumentOrderCommRate(&req, ++_reqId);
}

void CtpTdClient::OnRspQryInstrumentOrderCommRate(CThostFtdcInstrumentOrderCommRateField *pInstrumentOrderCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pInstrumentOrderCommRate, pRspInfo);

    if (bIsLast) _sem.release();
}
