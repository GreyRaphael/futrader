#include <ThostFtdcMdApi.h>

#include <memory>
#include <semaphore>
#include <string>
#include <vector>

struct MdClient : CThostFtdcMdSpi {
    MdClient(std::string_view cfg_file);
    ~MdClient();

    void Start();
    void Subscribe(std::vector<std::string> symbols);

   private:
    void OnFrontConnected() override;
    void OnFrontDisconnected(int nReason) override;
    void OnHeartBeatWarning(int nTimeLapse) override;
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;

    // pImpl pattern
    struct Impl;
    std::unique_ptr<Impl> pImpl{};

    CThostFtdcMdApi *_mdapi{};
    std::binary_semaphore _sem{0};
};