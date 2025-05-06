#include "tdclient.h"

#include <cstring>
#include <print>

void TdClient::OnFrontConnected() {
    std::println("OnFrontConnected");

    CThostFtdcReqAuthenticateField req{};
    strcpy(req.BrokerID, "broker_id");
    strcpy(req.UserID, user_id);
    strcpy(req.AppID, "const char *__restrict src");
    strcpy(req.AuthCode, "const char *__restrict src");
    _tdapi->ReqAuthenticate(&req, ++request_id);
}

void TdClient::OnFrontDisconnected(int nReason) {
    std::println("OnFrontDisconnected, reason={}", nReason);
}

void TdClient::OnHeartBeatWarning(int nTimeLapse) {
    std::println("OnHeartBeatWarning, time={}", nTimeLapse);
}

void TdClient::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::println("OnRspAuthenticate");
    if (bIsLast) {
        CThostFtdcReqUserLoginField req{};
        strcpy(req.BrokerID, "const char *__restrict src");
        strcpy(req.UserID, "const char *__restrict src");
        strcpy(req.Password, "const char *__restrict src");
        _tdapi->ReqUserLogin(&req, ++request_id);
    }
}

void TdClient::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::println("OnRspUserLogin");
    std::println("{} login at {} {}", pRspUserLogin->UserID, pRspUserLogin->TradingDay, pRspUserLogin->LoginTime);
    if (bIsLast) {
        CThostFtdcSettlementInfoConfirmField req{};
        strcpy(req.BrokerID, "const char *__restrict src");
        strcpy(req.InvestorID, "const char *__restrict src");
        _tdapi->ReqSettlementInfoConfirm(&req, ++request_id);
    }
}

void TdClient::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::println("OnRspSettlementInfoConfirm");
    if (bIsLast) {
        CThostFtdcQryInvestorPositionField req{};
        _tdapi->ReqQryInvestorPosition(&req, ++request_id);
    }
}

void TdClient::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::println("OnRspQryInvestorPosition");

    if (pInvestorPosition) {
        std::println("symbol={},position={}", pInvestorPosition->InstrumentID, pInvestorPosition->Position);
    } else {
        std::println("hold nothing");
    }

    if (bIsLast) {
        std::println("OnRspQryInvestorPosition done!");
    }
}