#include <ThostFtdcMdApi.h>

#include <memory>
#include <semaphore>
#include <string>
#include <vector>

#include "../common/spsc.hpp"

using MarketDataChannel = lockfree::SPSC<CThostFtdcDepthMarketDataField, 1024>;
using MarketDataChannelPtr = std::shared_ptr<MarketDataChannel>;

struct MdClient : CThostFtdcMdSpi {
    MdClient(std::string_view cfg_filename, MarketDataChannelPtr channel_ptr);
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
    MarketDataChannelPtr _channel_ptr;
};