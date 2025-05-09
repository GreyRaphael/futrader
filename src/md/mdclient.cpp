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

MdClient::MdClient(std::string_view cfg_file) : pImpl(std::make_unique<Impl>()) {
    // read toml config
    pImpl->cfg = read_config(cfg_file);
}

MdClient::~MdClient() { _mdapi->Release(); }

void MdClient::Start() {
    // load dylib
    auto dylib_path = std::format("{}/{}", pImpl->cfg.lib_dir, pImpl->cfg.platform);
    // inplace-construct the dylib inside the optional
    pImpl->lib.emplace(dylib_path, "thostmduserapi_se.so", dylib::no_filename_decorations);

    auto GetApiVersion = pImpl->lib->get_function<const char *()>("_ZN15CThostFtdcMdApi13GetApiVersionEv");
    std::println("md_ver={}", GetApiVersion());
    auto CreateFtdcMdApi = pImpl->lib->get_function<CThostFtdcMdApi *(const char *, const bool, const bool)>("_ZN15CThostFtdcMdApi15CreateFtdcMdApiEPKcbb");

    // register
    _mdapi = CreateFtdcMdApi("", false, false);
    _mdapi->RegisterSpi(this);
    _mdapi->RegisterFront(pImpl->cfg.front_md.data());

    // connect
    _mdapi->Init();
    _sem.acquire();

    // login
    CThostFtdcReqUserLoginField req{};
    pImpl->cfg.user_id.copy(req.UserID, pImpl->cfg.user_id.length());
    _mdapi->ReqUserLogin(&req, 1);
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
    std::vector<char *> symbol_pointers;
    symbol_pointers.reserve(64);
    for (auto &e : symbols) {
        symbol_pointers.push_back(e.data());
    }
    _mdapi->SubscribeMarketData(symbol_pointers.data(), symbol_pointers.size());
    _sem.acquire();
}

void MdClient::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pSpecificInstrument, pRspInfo);

    if (bIsLast) _sem.release();
}

void MdClient::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    print_struct(pDepthMarketData);
}