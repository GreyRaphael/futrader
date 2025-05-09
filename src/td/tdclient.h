#include <dylib.hpp>
#include <optional>
#include <semaphore>
#include <string>

#include "ThostFtdcTraderApi.h"

struct TdConfig {
    std::string mode;
    std::string platform;
    std::string front_td;
    std::string auth_id;
    std::string auth_code;
    std::string broker_id;
    std::string user_id;
    std::string password;
};

struct TdClient : CThostFtdcTraderSpi {
    TdClient() {}
    ~TdClient() { _tdapi->Release(); }

    void Start();
    void ReqSettlementInfo();
    void QryInvestorPosition();
    void QryTradingAccount();

   private:
    void OnFrontConnected() override;
    void OnFrontDisconnected(int nReason) override;
    void OnHeartBeatWarning(int nTimeLapse) override;
    void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    CThostFtdcTraderApi *_tdapi{};
    std::binary_semaphore _sem{0};
    std::optional<dylib> _lib_td{};
    int _reqId{0};
};