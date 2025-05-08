#include <dylib.hpp>
#include <map>
#include <optional>
#include <semaphore>
#include <string>
#include <vector>

#include "ThostFtdcMdApi.h"

struct MdConfig {
    std::string mode;
    std::string platform;
    std::string front_md;
    std::string user_id;
};

struct MdClient : CThostFtdcMdSpi {
    MdClient() {}
    ~MdClient() { _mdapi->Release(); }

    void Start();
    void Subscribe(std::vector<std::string> symbols);

   private:
    void OnFrontConnected() override;
    void OnFrontDisconnected(int nReason) override;
    void OnHeartBeatWarning(int nTimeLapse) override;
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;

    std::map<int, std::string> _err_map{};
    MdConfig _config{};
    CThostFtdcMdApi *_mdapi{};
    std::binary_semaphore _sem{0};
    // after invoked, dylib will unload the dll, so must set to field variable
    // dylib doesn't have a default constructor, so use std::optional or std::unique_ptr
    std::optional<dylib> _lib_md{};
};