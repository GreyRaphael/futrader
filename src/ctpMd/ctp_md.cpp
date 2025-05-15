#include "ctp_md.h"

#include <config_parser.h>

#include <dylib.hpp>
#include <error_parser.hpp>
#include <format>
#include <memory>
#include <optional>
#include <print>

struct CtpMdClient::Impl {
    CtpConfig cfg{};
    // after invoked, dylib will unload the dll, so must set to field variable
    // dylib doesn't have a default constructor, so use std::optional or std::unique_ptr
    std::optional<dylib> lib{};
};

CtpMdClient::CtpMdClient(std::string_view cfg_filename, MarketDataChannelPtr channel_ptr)
    : _pimpl(std::make_unique<Impl>()), _channel_ptr(channel_ptr) {
    // read toml config
    _pimpl->cfg = CtpConfig::read_config(cfg_filename);
}

CtpMdClient::~CtpMdClient() { _api->Release(); }

void CtpMdClient::Start() {
    // load dylib
    auto dylib_path = std::format("{}/{}", _pimpl->cfg.lib_dir, _pimpl->cfg.platform);
    // inplace-construct the dylib inside the optional
    _pimpl->lib.emplace(dylib_path, "thostmduserapi_se.so", dylib::no_filename_decorations);

    auto GetApiVersion = _pimpl->lib->get_function<const char *()>("_ZN15CThostFtdcMdApi13GetApiVersionEv");
    std::println("md_ver={}", GetApiVersion());
    auto CreateFtdcMdApi = _pimpl->lib->get_function<CThostFtdcMdApi *(const char *, const bool, const bool)>("_ZN15CThostFtdcMdApi15CreateFtdcMdApiEPKcbb");

    // register
    _api = CreateFtdcMdApi("", false, false);
    _api->RegisterSpi(this);
    _api->RegisterFront(_pimpl->cfg.front_md.data());

    // connect
    _api->Init();
    _sem.acquire();

    // login
    CThostFtdcReqUserLoginField req{};
    _pimpl->cfg.user_id.copy(req.UserID, _pimpl->cfg.user_id.length());
    _api->ReqUserLogin(&req, 1);
    _sem.acquire();
}

void CtpMdClient::OnFrontConnected() {
    std::println("OnFrontConnected");
    _sem.release();
}

void CtpMdClient::OnFrontDisconnected(int nReason) {
    std::println("OnFrontDisconnected: {}", errconfig::discon_errors.at(nReason));
}

void CtpMdClient::OnHeartBeatWarning(int nTimeLapse) {
    std::println("OnHeartBeatWarning: {}", nTimeLapse);
}

void CtpMdClient::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pRspUserLogin, pRspInfo);

    if (bIsLast) _sem.release();
};

void CtpMdClient::Subscribe(std::vector<std::string> symbols) {
    std::vector<char *> symbol_ptrs;
    symbol_ptrs.reserve(256);
    for (auto &e : symbols) {
        symbol_ptrs.push_back(e.data());
    }
    _api->SubscribeMarketData(symbol_ptrs.data(), symbol_ptrs.size());
    _sem.acquire();
}

void CtpMdClient::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pSpecificInstrument, pRspInfo);

    if (bIsLast) _sem.release();
}

void CtpMdClient::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    // print_struct(pDepthMarketData);
    // _channel_ptr->push(*pDepthMarketData);
}