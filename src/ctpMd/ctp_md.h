#include <ThostFtdcMdApi.h>

#include <memory>
#include <semaphore>
#include <spsc.hpp>
#include <string>
#include <vector>

#include "quotetype.h"

using TickDataChannel = lockfree::SPSC<TickData, 1024>;
using TickDataChannelPtr = std::shared_ptr<TickDataChannel>;

struct CtpMdClient : CThostFtdcMdSpi {
    CtpMdClient(std::string_view cfg_filename, TickDataChannelPtr channel_ptr);
    ~CtpMdClient();

    void Start();
    void Subscribe(std::vector<std::string> const &symbols);

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
    TickDataChannelPtr _channel_ptr;
};