#include "ctp_md.h"

#include <config_parser.h>

#include <cassert>
#include <cstring>
#include <dylib.hpp>
#include <error_parser.hpp>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <print>

#include "quotetype.h"
#include "tick_parser.hpp"

struct CtpMdClient::Impl {
    CtpConfig cfg{};
    // after invoked, dylib will unload the dll, so must set to field variable
    // dylib doesn't have a default constructor, so use std::optional or std::unique_ptr
    std::optional<dylib> lib{};
};

CtpMdClient::CtpMdClient(std::string_view cfg_filename, MarketDataChannelPtr channel_ptr)
    : _pimpl(std::make_unique<Impl>()), _channel_ptr(channel_ptr) {
    // assert broker.toml exist
    assert(std::filesystem::exists(cfg_filename));
    assert(std::filesystem::exists("errors.toml"));
    // read toml config
    _pimpl->cfg = CtpConfig::read_config(cfg_filename, "md");
    // assert *.so exist
    assert(std::filesystem::exists(_pimpl->cfg.Interface));
}

CtpMdClient::~CtpMdClient() { _api->Release(); }

void CtpMdClient::Start() {
    // load dylib
    auto dylib_path = std::filesystem::path{_pimpl->cfg.Interface};
    // inplace-construct the dylib inside the optional
    _pimpl->lib.emplace(dylib_path.parent_path().c_str(), dylib_path.filename().c_str(), dylib::no_filename_decorations);

    auto GetApiVersion = _pimpl->lib->get_function<const char *()>("_ZN15CThostFtdcMdApi13GetApiVersionEv");
    std::println("md_ver={}", GetApiVersion());
    auto CreateFtdcMdApi = _pimpl->lib->get_function<CThostFtdcMdApi *(const char *, const bool, const bool)>("_ZN15CThostFtdcMdApi15CreateFtdcMdApiEPKcbb");

    // register
    _api = CreateFtdcMdApi("", false, false);
    _api->RegisterSpi(this);
    _api->RegisterFront(_pimpl->cfg.Front.data());

    // connect
    _api->Init();
    _sem.acquire();

    // login
    CThostFtdcReqUserLoginField req{};
    _pimpl->cfg.UserID.copy(req.UserID, _pimpl->cfg.UserID.length());
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

void CtpMdClient::Subscribe(std::vector<std::string> const &symbols) {
    std::vector<char *> symbol_ptrs;
    symbol_ptrs.reserve(256);
    for (auto &e : symbols) {
        symbol_ptrs.push_back(const_cast<char *>(e.data()));
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

    TickData tick{};
    strncpy(tick.symbol, pDepthMarketData->InstrumentID, 31);
    auto dt_str = std::format("{} {}", pDepthMarketData->ActionDay, pDepthMarketData->UpdateTime);
    auto tp = str2tp(dt_str, "%Y%m%d %H:%M:%S");
    tick.stamp = (tp + std::chrono::milliseconds(pDepthMarketData->UpdateMillisec)).time_since_epoch().count();
    tick.open = pDepthMarketData->OpenPrice;
    tick.high = pDepthMarketData->HighestPrice;
    tick.low = pDepthMarketData->LowestPrice;
    tick.last = pDepthMarketData->LastPrice;
    tick.close = pDepthMarketData->ClosePrice;
    tick.settle = pDepthMarketData->SettlementPrice;
    tick.limit_up = pDepthMarketData->UpperLimitPrice;
    tick.limit_down = pDepthMarketData->UpperLimitPrice;
    tick.preclose = pDepthMarketData->PreClosePrice;
    tick.presettle = pDepthMarketData->PreSettlementPrice;
    tick.volume = pDepthMarketData->Volume;
    tick.amount = pDepthMarketData->Turnover;
    tick.oi = pDepthMarketData->OpenInterest;
    tick.preoi = pDepthMarketData->PreOpenInterest;
    tick.avgprice = pDepthMarketData->AveragePrice;
    tick.adj = 1.0;  // todo
    tick.ap1 = pDepthMarketData->AskPrice1;
    tick.ap2 = pDepthMarketData->AskPrice2;
    tick.ap3 = pDepthMarketData->AskPrice3;
    tick.ap4 = pDepthMarketData->AskPrice4;
    tick.ap5 = pDepthMarketData->AskPrice5;
    tick.bp1 = pDepthMarketData->BidPrice1;
    tick.bp2 = pDepthMarketData->BidPrice2;
    tick.bp3 = pDepthMarketData->BidPrice3;
    tick.bp4 = pDepthMarketData->BidPrice4;
    tick.bp5 = pDepthMarketData->BidPrice5;
    tick.av1 = pDepthMarketData->AskVolume1;
    tick.av2 = pDepthMarketData->AskVolume2;
    tick.av3 = pDepthMarketData->AskVolume3;
    tick.av4 = pDepthMarketData->AskVolume4;
    tick.av5 = pDepthMarketData->AskVolume5;
    tick.bv1 = pDepthMarketData->BidVolume1;
    tick.bv2 = pDepthMarketData->BidVolume2;
    tick.bv3 = pDepthMarketData->BidVolume3;
    tick.bv4 = pDepthMarketData->BidVolume4;
    tick.bv5 = pDepthMarketData->BidVolume5;
    _channel_ptr->push(tick);
}