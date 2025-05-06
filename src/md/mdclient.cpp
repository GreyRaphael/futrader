#include "mdclient.h"

#include <cstring>
#include <print>
#include <vector>

void MdClient::OnFrontConnected() {
    std::println("OnFrontConnected");

    CThostFtdcReqUserLoginField req{};
    strcpy(req.UserID, user_id);
    _mdapi->ReqUserLogin(&req, 1);
}

void MdClient::OnFrontDisconnected(int nReason) {
    std::println("OnFrontDisconnected, reason={}", nReason);
}

void MdClient::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::println("OnRspUserLogin");
    std::println("login at {}", pRspUserLogin->LoginTime);
    if (bIsLast) {
        std::vector<char *> symbols{"MA509", "rb2509"};
        int count = symbols.size();
        _mdapi->SubscribeMarketData(symbols.data(), count);
    }
};

void MdClient::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::println("OnRspSubMarketData");
    std::println("sub {}", pSpecificInstrument->InstrumentID);
}

void MdClient::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
    std::println("{}, px={}, at {} {} {}", pDepthMarketData->InstrumentID, pDepthMarketData->LastPrice, pDepthMarketData->ActionDay, pDepthMarketData->UpdateTime, pDepthMarketData->UpdateMillisec);
}