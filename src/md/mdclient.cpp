#include "mdclient.h"

#include <toml++/toml.h>

#include <cstring>
#include <dylib.hpp>
#include <format>
#include <map>
#include <print>
#include <string>
#include <utils/parser.hpp>
#include <utils/repr.hpp>
#include <vector>

MdConfig ReadConfig(std::string_view filename) {
    auto config = toml::parse_file(filename);
    std::string mode = config["default"]["mode"].value_or("openctp");
    std::string platform = config["default"]["platform"].value_or("lin64");
    std::string front_md = config[mode]["front_md"].value_or("");
    std::string user_id = config[mode]["user_id"].value_or("");
    return MdConfig{mode, platform, front_md, user_id};
}

void MdClient::Start() {
    // read error mapping
    _err_map = load_errors("errors.toml");

    // read config
    _config = ReadConfig("config.toml");

    // load dylib
    auto dylib_path = std::format("{}/{}", _config.mode, _config.platform);
    // inplace-construct the dylib inside the optional
    _lib_md.emplace(dylib_path, "thostmduserapi_se.so", dylib::no_filename_decorations);
    auto GetApiVersion = _lib_md->get_function<const char *()>("_ZN15CThostFtdcMdApi13GetApiVersionEv");
    std::println("md_ver={}", GetApiVersion());
    auto CreateFtdcMdApi = _lib_md->get_function<CThostFtdcMdApi *(const char *, const bool, const bool)>("_ZN15CThostFtdcMdApi15CreateFtdcMdApiEPKcbb");

    // register
    _mdapi = CreateFtdcMdApi("", false, false);
    _mdapi->RegisterSpi(this);
    _mdapi->RegisterFront(_config.front_md.data());

    // connect
    _mdapi->Init();
    _sem.acquire();

    // login
    CThostFtdcReqUserLoginField req{};
    _config.user_id.copy(req.UserID, _config.user_id.length());
    _mdapi->ReqUserLogin(&req, 1);
    _sem.acquire();
}

void MdClient::OnFrontConnected() {
    std::println("OnFrontConnected");
    _sem.release();
}

std::map<int, std::string> DisconnectionMap{
    {0x1001, "网络读失败"},
    {0x1002, "网络写失败"},
    {0x2001, "接收心跳超时"},
    {0x2002, "发送心跳失败"},
    {0x2003, "收到错误报文"},
};

void MdClient::OnFrontDisconnected(int nReason) {
    std::println("OnFrontDisconnected: {}", DisconnectionMap[nReason]);
}

void MdClient::OnHeartBeatWarning(int nTimeLapse) {
    std::println("OnHeartBeatWarning: {}", nTimeLapse);
}

void MdClient::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    if (0 != pRspInfo->ErrorID) {
        std::println("OnRspUserLogin, {}", _err_map[pRspInfo->ErrorID]);
        return;
    }

    if (pRspUserLogin) {
        print_struct(pRspUserLogin);
    }

    if (bIsLast) {
        _sem.release();
    }
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
    if (0 != pRspInfo->ErrorID) {
        std::println("OnRspSubMarketData, {}", _err_map[pRspInfo->ErrorID]);
        return;
    }

    if (pSpecificInstrument) {
        print_struct(pSpecificInstrument);
    }

    if (bIsLast) {
        _sem.release();
    }
}

void MdClient::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    print_struct(pDepthMarketData);
}