#include "ThostFtdcMdApi.h"

struct MdClient : CThostFtdcMdSpi {
    MdClient(CThostFtdcMdApi *api, const char *uid) : _mdapi(api), user_id(uid) {}
    ~MdClient() {}

    void OnFrontConnected() override;
    void OnFrontDisconnected(int nReason) override;
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;

    CThostFtdcMdApi *_mdapi{};
    const char *user_id{};
};