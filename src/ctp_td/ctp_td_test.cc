#include <chrono>
#include <string_view>
#include <thread>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>

#include "ctp_td.h"

TEST_CASE("tts") {
    std::string_view cfg_filename{"tts.toml"};
    REQUIRE(std::filesystem::exists(cfg_filename));
    REQUIRE(std::filesystem::exists("errors.toml"));
    REQUIRE(std::filesystem::exists("tts/thosttraderapi_se.so"));

    CtpTdClient td_cli{cfg_filename};
    td_cli.Start();
    // td_cli.QryInstrument({"SHFE", "INE", "CZCE"});
    // td_cli.QryInstrument({});
    // td_cli.QryExchange();
    // td_cli.QryProduct();
    // td_cli.QryInstrumentCommissionRate();
    // td_cli.QryInstrumentOrderCommRate();
    // td_cli.SettlementInfo();
    // td_cli.QryTradingAccount();
    // td_cli.QryInvestorPosition();
    // td_cli.QryDepthMarketData("rb2507");
    td_cli.QryDepthMarketData("rb2510");
    getchar();
}

TEST_CASE("ctp") {
    std::string_view cfg_filename{"ctp.toml"};
    REQUIRE(std::filesystem::exists(cfg_filename));
    REQUIRE(std::filesystem::exists("errors.toml"));
    REQUIRE(std::filesystem::exists("ctp/thosttraderapi_se.so"));
    CtpTdClient td_cli{cfg_filename};
    td_cli.Start();
    // td_cli.QryInstrument({"SHFE", "INE", "CZCE"});
    // td_cli.QryInstrument({});
    // td_cli.QryExchange();
    // td_cli.QryProduct();
    // td_cli.QryInstrumentCommissionRate();
    // td_cli.QryInstrumentOrderCommRate();
    // td_cli.SettlementInfo();
    // td_cli.QryTradingAccount();
    // td_cli.QryInvestorPosition();
    // td_cli.QryDepthMarketData("rb2507");

    // 每秒查询10-15次的流速控制，超速了返回-3
    for (auto i = 0; i < 10; ++i) {
        td_cli.QryDepthMarketData("rb2507");
        td_cli.QryDepthMarketData("rb2510");
    }

    getchar();
}