#include <chrono>
#include <cstdio>
#include <memory>
#include <string_view>
#include <thread>

#include "quote_type.h"
#include "spsc.hpp"
#include "struct_parser.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include <print>

#include "ctp_md.hpp"

TEST_CASE("tts") {
    std::string_view cfg_filename{"tts.toml"};
    REQUIRE(std::filesystem::exists(cfg_filename));
    REQUIRE(std::filesystem::exists("errors.toml"));
    REQUIRE(std::filesystem::exists("tts/thostmduserapi_se.so"));

    CtpMdClient md_cli{cfg_filename, [](TickData const& tick) {
                           print_struct(&tick);
                           // stamp to time_point
                           auto tp = std::chrono::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds{tick.stamp}};
                           std::println("{} {} {}", tick.symbol, tp, tick.last);
                       }};
    md_cli.subscribe({"MA509", "rb2507"});
    md_cli.start();

    getchar();
}

TEST_CASE("ctp") {
    std::string_view cfg_filename{"ctp.toml"};
    REQUIRE(std::filesystem::exists(cfg_filename));
    REQUIRE(std::filesystem::exists("errors.toml"));
    REQUIRE(std::filesystem::exists("ctp/thostmduserapi_se.so"));

    CtpMdClient md_cli{cfg_filename, [](TickData const& tick) {
                           // print_struct(&tick);
                           auto tp = std::chrono::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds{tick.stamp}};
                           std::println("{} {} {}", tick.symbol, tp, tick.last);
                       }};
    md_cli.subscribe({"MA509", "rb2507"});
    md_cli.start();

    getchar();
}

using TickDataChannel = lockfree::SPSC<TickData, 1024>;
using TickDataChannelPtr = std::shared_ptr<TickDataChannel>;

TEST_CASE("tts_spsc_heap") {
    constexpr std::string_view CFG_FILENAME{"tts.toml"};
    REQUIRE(std::filesystem::exists(CFG_FILENAME));
    REQUIRE(std::filesystem::exists("errors.toml"));
    REQUIRE(std::filesystem::exists("tts/thostmduserapi_se.so"));

    // SPSC on heap
    auto channel_ptr = std::make_shared<TickDataChannel>();

    // market data producer
    CtpMdClient md_cli{
        CFG_FILENAME,
        [channel_ptr](const TickData& tick) noexcept {  // value-copy duplice shared_ptr
            // non-blocking push; drop on overflow or handle return if needed
            channel_ptr->push(tick);
        }};

    // market data producer
    std::jthread md_thread{[&md_cli] {
        md_cli.subscribe({"MA509", "rb2507"});
        md_cli.start();  // blocking call
    }};

    // main thread consumer
    while (true) {
        auto value = channel_ptr->pop();

        if (value) {
            auto tp = std::chrono::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds{value->stamp}};
            std::println("{},{},{}", value->symbol, tp, value->last);
            // print_struct(&value.value());
        } else {
            // back off when empty
            std::println("channel empty, retrying...");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
}

// of high performance
TEST_CASE("tts_spsc_stack") {
    constexpr std::string_view CFG_FILENAME{"tts.toml"};
    REQUIRE(std::filesystem::exists(CFG_FILENAME));
    REQUIRE(std::filesystem::exists("errors.toml"));
    REQUIRE(std::filesystem::exists("tts/thostmduserapi_se.so"));

    // Use single stack-based channel without heap allocation
    TickDataChannel channel;

    // market data producer thread
    CtpMdClient md_cli{
        CFG_FILENAME,
        [&channel](const TickData& tick) noexcept {
            // non-blocking push; drop on overflow or handle return if needed
            channel.push(tick);
        }};

    // market data producer
    std::jthread md_thread{[&md_cli] {
        md_cli.subscribe({"MA509", "rb2507"});
        md_cli.start();  // blocking call
    }};

    // main thread consumer
    TickData tick{};
    while (true) {
        if (channel.pop(tick)) {
            auto tp = std::chrono::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds{tick.stamp}};
            std::println("{},{},{}", tick.symbol, tp, tick.last);
            // print_struct(&value.value());
        } else {
            // back off when empty
            std::println("channel empty, retrying...");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
}