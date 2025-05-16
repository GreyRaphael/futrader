#include <string_view>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>

#include "ctp_td.h"

TEST_CASE("openctp") {
    std::string_view cfg_filename{"openctp.toml"};
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
    td_cli.QryTradingAccount();
    td_cli.QryInvestorPosition();
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
    td_cli.QryTradingAccount();
    td_cli.QryInvestorPosition();
    getchar();
}