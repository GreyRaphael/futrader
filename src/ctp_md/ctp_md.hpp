#pragma once
#include <ThostFtdcMdApi.h>

#include <algorithm>
#include <cstring>
#include <dylib.hpp>
#include <optional>
#include <print>
#include <semaphore>
#include <string>
#include <vector>

#include "config_parser.h"
#include "error_parser.hpp"
#include "quote_type.h"
#include "tick_parser.hpp"

template <typename CB>
struct CtpMdClient : CThostFtdcMdSpi {
    CtpMdClient(std::string_view cfg_filename, CB callback) : _callback(std::move(callback)) {
        // assert broker.toml exist
        assert(std::filesystem::exists(cfg_filename));
        assert(std::filesystem::exists("errors.toml"));
        // read toml config
        _cfg = CtpConfig::read_config(cfg_filename, "md");
        // assert *.so exist
        assert(std::filesystem::exists(_cfg.Interface));
    }
    ~CtpMdClient() { _api->Release(); }

    void subscribe(std::vector<std::string> symbols) { _symbols = std::move(symbols); }
    void start();

   private:
    void OnFrontConnected() override;
    void OnFrontDisconnected(int nReason) override;
    void OnHeartBeatWarning(int nTimeLapse) override;
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;

    CtpConfig _cfg{};
    CB _callback{};
    // after invoked, dylib will unload the dll, so must set to field variable
    // dylib doesn't have a default constructor, so use std::optional or std::unique_ptr
    std::optional<dylib> _lib{};
    CThostFtdcMdApi *_api{};
    std::binary_semaphore _sem{0};
    std::vector<std::string> _symbols;
    TimeStampCalculator _stamp_calculator{};
};

template <typename CB>
void CtpMdClient<CB>::start() {
    // load dylib
    auto dylib_path = std::filesystem::path{_cfg.Interface};
    // inplace-construct the dylib inside the optional
    _lib.emplace(dylib_path.parent_path().c_str(), dylib_path.filename().c_str(), dylib::no_filename_decorations);

    auto GetApiVersion = _lib->get_function<const char *()>("_ZN15CThostFtdcMdApi13GetApiVersionEv");
    std::println("md_ver={}", GetApiVersion());
    auto CreateFtdcMdApi = _lib->get_function<CThostFtdcMdApi *(const char *, const bool, const bool)>("_ZN15CThostFtdcMdApi15CreateFtdcMdApiEPKcbb");

    // register
    _api = CreateFtdcMdApi("", false, false);
    _api->RegisterSpi(this);
    _api->RegisterFront(_cfg.Front.data());

    // connect
    _api->Init();
    _sem.acquire();

    // login
    CThostFtdcReqUserLoginField req{};
    _cfg.UserID.copy(req.UserID, _cfg.UserID.length());
    _api->ReqUserLogin(&req, 0);
    _sem.acquire();

    // subscribe
    std::vector<char *> symbol_ptrs;
    symbol_ptrs.reserve(_symbols.size());
    for (auto &&e : _symbols) {
        symbol_ptrs.push_back(e.data());
    }

    _api->SubscribeMarketData(symbol_ptrs.data(), symbol_ptrs.size());
    _sem.acquire();
}

template <typename CB>
void CtpMdClient<CB>::OnFrontConnected() {
    std::println("OnFrontConnected");
    _sem.release();
}

template <typename CB>
void CtpMdClient<CB>::OnFrontDisconnected(int nReason) {
    std::println("OnFrontDisconnected: {}", errconfig::discon_errors.at(nReason));
}

template <typename CB>
void CtpMdClient<CB>::OnHeartBeatWarning(int nTimeLapse) {
    std::println("OnHeartBeatWarning: {}", nTimeLapse);
}

template <typename CB>
void CtpMdClient<CB>::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pRspUserLogin, pRspInfo);

    if (bIsLast) _sem.release();
};

template <typename CB>
void CtpMdClient<CB>::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    handle_resp(pSpecificInstrument, pRspInfo);

    if (bIsLast) _sem.release();
}

template <typename CB>
void CtpMdClient<CB>::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    // print_struct(pDepthMarketData);

    TickData tick{};
    memcpy(tick.symbol, pDepthMarketData->InstrumentID, 31);
    // ActionDay or TradingDay?
    tick.stamp = _stamp_calculator.compute(pDepthMarketData->ActionDay, pDepthMarketData->UpdateTime, pDepthMarketData->UpdateMillisec);
    tick.open = pDepthMarketData->OpenPrice;
    tick.high = pDepthMarketData->HighestPrice;
    tick.low = pDepthMarketData->LowestPrice;
    tick.last = pDepthMarketData->LastPrice;
    tick.limit_down = pDepthMarketData->LowerLimitPrice;
    tick.limit_up = pDepthMarketData->UpperLimitPrice;
    tick.preclose = pDepthMarketData->PreClosePrice;
    tick.close = pDepthMarketData->ClosePrice;
    tick.presettle = pDepthMarketData->PreSettlementPrice;
    tick.settle = pDepthMarketData->SettlementPrice;
    tick.preoi = pDepthMarketData->PreOpenInterest;
    tick.oi = pDepthMarketData->OpenInterest;
    tick.volume = pDepthMarketData->Volume;
    tick.amount = pDepthMarketData->Turnover;
    tick.avgprice = pDepthMarketData->AveragePrice;
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
    tick.adj = 1.0;  // todo

    _callback(tick);
}
