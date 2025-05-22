#include <ThostFtdcMdApi.h>

#include <functional>
#include <memory>
#include <semaphore>
#include <string>
#include <vector>

#include "quote_type.h"

struct MdApiReleaser {
    void operator()(CThostFtdcMdApi *p) const noexcept {
        if (p) p->Release();  // or whatever the correct destroy call is
    }
};

struct CtpMdClient : CThostFtdcMdSpi {
    CtpMdClient(std::string_view cfg_filename);
    ~CtpMdClient() = default;

    void setCallback(std::function<void(const TickData &)> callback);
    void subscribe(std::vector<std::string> const &symbols);
    void start();

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

    std::unique_ptr<CThostFtdcMdApi, MdApiReleaser> _api{};
    std::binary_semaphore _sem{0};
    std::function<void(const TickData &)> _callback{};
};