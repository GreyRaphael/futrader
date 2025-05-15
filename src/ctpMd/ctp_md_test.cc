#include <cstdio>
#include <string_view>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include <print>

#include "ctp_md.h"

TEST_CASE("openctp") {
    std::string_view cfg_filename{"openctp.toml"};
    REQUIRE(std::filesystem::exists(cfg_filename));
    REQUIRE(std::filesystem::exists("errors.toml"));
    REQUIRE(std::filesystem::exists("openctp/lin64/thostmduserapi_se.so"));

    auto channel_ptr = std::make_shared<MarketDataChannel>();

    CtpMdClient md_cli{cfg_filename, channel_ptr};
    md_cli.Start();
    md_cli.Subscribe({"MA505", "rb2507"});

    while (true) {
        auto value = channel_ptr->pop();
        if (!value) {
            std::println("empty, cannot pop");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }
        // print_struct(&value);
        std::println("{},{},{}", value->UpdateTime, value->InstrumentID, value->LastPrice);
    }
}

TEST_CASE("ctp") {
    std::string_view cfg_filename{"ctp.toml"};
    REQUIRE(std::filesystem::exists(cfg_filename));
    REQUIRE(std::filesystem::exists("errors.toml"));
    REQUIRE(std::filesystem::exists("ctp/lin64/thostmduserapi_se.so"));

    auto channel_ptr = std::make_shared<MarketDataChannel>();

    CtpMdClient md_cli{cfg_filename, channel_ptr};
    md_cli.Start();
    md_cli.Subscribe({"MA505", "rb2507"});

    while (true) {
        auto value = channel_ptr->pop();
        if (!value) {
            std::println("empty, cannot pop");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }
        // print_struct(&value);
        std::println("{},{},{}", value->UpdateTime, value->InstrumentID, value->LastPrice);
    }
}