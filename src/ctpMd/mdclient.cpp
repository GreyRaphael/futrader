#include "mdclient.h"

#include <dylib.hpp>
#include <format>
#include <memory>
#include <optional>

#include "../utils.hpp"

struct MdClient::Impl {
    CtpConfig cfg{};
    // after invoked, dylib will unload the dll, so must set to field variable
    // dylib doesn't have a default constructor, so use std::optional or std::unique_ptr
    std::optional<dylib> lib{};
};

MdClient::MdClient(std::string_view cfg_filename, MarketDataChannelPtr channel_ptr)
    : _pimpl(std::make_unique<Impl>()), _channel_ptr(channel_ptr) {
    // read toml config
    _pimpl->cfg = read_config(cfg_filename);
}

MdClient::~MdClient() { _api->Release(); }

void MdClient::Start() {
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

void MdClient::OnFrontConnected() {
    std::println("OnFrontConnected");
    _sem.release();
}

void MdClient::OnFrontDisconnected(int nReason) {
    std::println("OnFrontDisconnected: {}", errconfig::discon_errors.at(nReason));
}

void MdClient::OnHeartBeatWarning(int nTimeLapse) {
    std::println("OnHeartBeatWarning: {}", nTimeLapse);
}

void MdClient::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pRspUserLogin, pRspInfo);

    if (bIsLast) _sem.release();
};

void MdClient::Subscribe(std::vector<std::string> symbols) {
    std::vector<char *> symbol_ptrs;
    symbol_ptrs.reserve(256);
    for (auto &e : symbols) {
        symbol_ptrs.push_back(e.data());
    }
    _api->SubscribeMarketData(symbol_ptrs.data(), symbol_ptrs.size());
    _sem.acquire();
}

void MdClient::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pSpecificInstrument, pRspInfo);

    if (bIsLast) _sem.release();
}

void MdClient::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    // print_struct(pDepthMarketData);
    _channel_ptr->push(*pDepthMarketData);
}