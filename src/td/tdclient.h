#include <ThostFtdcTraderApi.h>

#include <memory>
#include <semaphore>
#include <string_view>

struct TdClient : CThostFtdcTraderSpi {
    TdClient(std::string_view cfg_file);
    ~TdClient();

    void Start();

    void SettlementInfo();
    void QryInvestorPosition();
    void QryTradingAccount();
    void OrderAction();

    bool Buy(std::string_view symbol, int lot, double price);
    bool Sell(std::string_view symbol, int lot, double price);
    bool SellShort(std::string_view symbol, int lot, double price);
    bool Buy2Cover(std::string_view symbol, int lot, double price);

   private:
    void OrderInsert(std::string_view symbol, TThostFtdcDirectionType direction, TThostFtdcOffsetFlagType offset, TThostFtdcPriceType price, TThostFtdcVolumeType lot, bool is_stop);

   private:
    void OnFrontConnected() override;
    void OnFrontDisconnected(int nReason) override;
    void OnHeartBeatWarning(int nTimeLapse) override;
    void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
    void OnRtnOrder(CThostFtdcOrderField *pOrder) override;
    void OnRtnTrade(CThostFtdcTradeField *pTrade) override;

    void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    // pImpl pattern
    struct Impl;
    std::unique_ptr<Impl> pImpl{};

    CThostFtdcTraderApi *_tdapi{};
    std::binary_semaphore _sem{0};
    int _reqId{0};
};