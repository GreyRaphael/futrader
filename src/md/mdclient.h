#include <ThostFtdcMdApi.h>

#include <memory>
#include <semaphore>
#include <string>
#include <utils/spsc.hpp>
#include <vector>

using MarketDataQueue = lockfree::SPSC<CThostFtdcDepthMarketDataField, 1024>;
using MarketDataQueuePtr = std::shared_ptr<MarketDataQueue>;

struct MdClient : CThostFtdcMdSpi {
    MdClient(std::string_view cfg_filename, MarketDataQueuePtr queue_ptr);
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

    // PIMPL pattern
    struct Impl;
    std::unique_ptr<Impl> _pimpl{};

    CThostFtdcMdApi *_api{};
    std::binary_semaphore _sem{0};
    MarketDataQueuePtr _queue_ptr;
};