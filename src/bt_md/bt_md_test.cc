#include <chrono>
#include <filesystem>
#include <print>
#include <string_view>
#include <thread>

#include "struct_parser.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <bt_md.hpp>
#include <spsc.hpp>

TEST_CASE("duckdb") {
    std::string_view cfg_filename{"bt.toml"};
    REQUIRE(std::filesystem::exists(cfg_filename));

    BactestMdClient bt_cli{cfg_filename, [](TickData const& tick) {
                               // print_struct(&tick);
                               auto tp = std::chrono::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds{tick.stamp}};
                               std::println("{} {} {}", tick.symbol, tp, tick.last);
                           }};
    bt_cli.subscribe({"MA505", "rb2505"});
    bt_cli.start();
}

using TickDataChannel = lockfree::SPSC<TickData, 1024>;

TEST_CASE("duckdb_spsc") {
    // db to SPSC is bad, as db will make SPSC full
    std::string_view cfg_filename{"bt.toml"};
    REQUIRE(std::filesystem::exists(cfg_filename));

    TickDataChannel channel;

    BactestMdClient bt_cli{cfg_filename, [&channel](TickData const& tick) {
                               channel.push(tick);
                           }};
    std::jthread bt_thread{[&bt_cli] {
        bt_cli.subscribe({"MA505", "rb2505"});
        bt_cli.start();
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
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}