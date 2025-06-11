#pragma once
#include <ThostFtdcTraderApi.h>

#include <cassert>
#include <dylib.hpp>
#include <filesystem>
#include <optional>
#include <print>
#include <semaphore>
#include <string_view>

#include "config_parser.h"
#include "error_parser.hpp"

struct CtpTdClient : CThostFtdcTraderSpi {
    CtpTdClient(std::string_view cfg_filename) {
        assert(std::filesystem::exists(cfg_filename));
        assert(std::filesystem::exists("errors.toml"));
    }
    ~CtpTdClient() { _api->Release(); }

   public:
    void start();
    void orderInsert(std::string_view symbol, TThostFtdcDirectionType direction, TThostFtdcOffsetFlagType offset, TThostFtdcPriceType price, TThostFtdcVolumeType lot, bool is_stop);

   private:
    void OnFrontConnected() override;
    void OnFrontDisconnected(int nReason) override;
    void OnHeartBeatWarning(int nTimeLapse) override;
    void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

   private:
    CtpConfig _cfg{};
    std::optional<dylib> _lib{};
    CThostFtdcTraderApi *_api{};
    std::binary_semaphore _sem{0};
    int _req_id{0};
};

void CtpTdClient::start() {
    // load dylib
    auto dylib_path = std::filesystem::path{_cfg.Interface};
    _lib.emplace(dylib_path.parent_path().c_str(), dylib_path.filename().c_str(), dylib::no_filename_decorations);

    auto GetApiVersion = _lib->get_function<const char *()>("_ZN19CThostFtdcTraderApi13GetApiVersionEv");
    std::println("td_ver={}", GetApiVersion());
    auto CreateFtdcTraderApi = _lib->get_function<CThostFtdcTraderApi *(const char *)>("_ZN19CThostFtdcTraderApi19CreateFtdcTraderApiEPKc");

    // register
    _api = CreateFtdcTraderApi("");
    _api->RegisterSpi(this);
    _api->RegisterFront(_cfg.Front.data());

    // connect
    _api->Init();
    _sem.acquire();

    // auth
    CThostFtdcReqAuthenticateField auth_req{};
    _cfg.BrokerID.copy(auth_req.BrokerID, _cfg.BrokerID.length());
    _cfg.UserID.copy(auth_req.UserID, _cfg.UserID.length());
    _cfg.AppID.copy(auth_req.AppID, _cfg.AppID.length());
    _cfg.AuthCode.copy(auth_req.AuthCode, _cfg.AuthCode.length());
    _api->ReqAuthenticate(&auth_req, ++_req_id);
    _sem.acquire();

    // login
    CThostFtdcReqUserLoginField login_req{};
    _cfg.BrokerID.copy(login_req.BrokerID, _cfg.BrokerID.length());
    _cfg.UserID.copy(login_req.UserID, _cfg.UserID.length());
    _cfg.Password.copy(login_req.Password, _cfg.Password.length());
    _api->ReqUserLogin(&login_req, ++_req_id);
    _sem.acquire();
}

void CtpTdClient::OnFrontConnected() {
    std::println("OnFrontConnected");
    _sem.release();
}

void CtpTdClient::OnFrontDisconnected(int nReason) {
    std::println("OnFrontDisconnected: {}", errconfig::DISCON_ERRORS.at(nReason));
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

void CtpTdClient::orderInsert(std::string_view symbol, TThostFtdcDirectionType direction, TThostFtdcOffsetFlagType offset, TThostFtdcPriceType price, TThostFtdcVolumeType lot, bool is_stop) {
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
    _api->ReqOrderInsert(&req, ++_req_id);
}
