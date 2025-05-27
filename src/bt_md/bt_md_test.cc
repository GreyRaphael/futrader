#include <chrono>
#include <cstddef>
#include <filesystem>
#include <print>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "struct_parser.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <zmq.h>

#include <bt_md.hpp>
#include <spsc.hpp>

TEST_CASE("duckdb") {
    std::string_view cfg_filename{"bt.toml"};
    REQUIRE(std::filesystem::exists(cfg_filename));

    size_t count = 0;
    BactestMdClient bt_cli{cfg_filename, [&count](TickData const& tick) {
                               // print_struct(&tick);
                               auto tp = std::chrono::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds{tick.stamp}};
                               std::println("{} {} {}", tick.symbol, tp, tick.last);
                               ++count;
                           }};
    bt_cli.subscribe({"MA505", "rb2505"});
    bt_cli.start();
    std::println("Total ticks processed: {}", count);
}

using TickDataChannel = lockfree::SPSC<TickData, 1024>;

TEST_CASE("duckdb_spsc") {
    // db to SPSC is bad, as db will make SPSC full
    std::string_view cfg_filename{"bt.toml"};
    REQUIRE(std::filesystem::exists(cfg_filename));

    TickDataChannel channel;

    BactestMdClient bt_cli{cfg_filename, [&channel](TickData const& tick) {
                               while (!channel.push(tick)) {
                                   std::println("channel full, cannot push tick, retrying...");
                                   std::this_thread::sleep_for(std::chrono::milliseconds(10));
                               }
                           }};
    std::jthread bt_thread{[&bt_cli] {
        bt_cli.subscribe({"MA505", "rb2505"});
        bt_cli.start();
    }};

    // main thread consumer
    TickData tick{};
    size_t count = 0;
    while (true) {
        if (channel.pop(tick)) {
            auto tp = std::chrono::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds{tick.stamp}};
            std::println("{},{},{}", tick.symbol, tp, tick.last);
            ++count;
            // print_struct(&value.value());
        } else {
            // back off when empty
            std::println("processed {} ticks, channel empty, retrying...", count);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

TEST_CASE("duckdb_zmq_send") {
    std::string_view cfg_filename = "zmq.toml";
    REQUIRE(std::filesystem::exists(cfg_filename));
    auto config = ZmqConfig::readConfig(cfg_filename);

    void* context = zmq_ctx_new();
    void* publisher = zmq_socket(context, ZMQ_PUB);

    // Set high-water mark for outbound messages
    int sndhwm = 0;  // Adjust as needed
    zmq_setsockopt(publisher, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));

    // Bind to IPC address (e.g., "ipc:///tmp/your_socket")
    zmq_bind(publisher, config.address.c_str());

    BactestMdClient bt_cli{config.broker_file, [publisher](TickData& tick) {
                               zmq_send(publisher, &tick, sizeof(TickData), 0);
                           }};

    bt_cli.subscribe(config.symbols);
    bt_cli.start();

    // Cleanup (will not be reached if bt_cli.start() blocks indefinitely)
    zmq_close(publisher);
    zmq_ctx_term(context);
}

TEST_CASE("duckdb_zmq_recv") {
    std::string_view cfg_filename = "zmq.toml";
    REQUIRE(std::filesystem::exists(cfg_filename));
    auto config = ZmqConfig::readConfig(cfg_filename);

    void* context = zmq_ctx_new();
    void* subscriber = zmq_socket(context, ZMQ_SUB);

    // Set high-water mark for inbound messages
    int rcvhwm = 0;  // Adjust as needed
    zmq_setsockopt(subscriber, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm));

    // Connect to IPC address (e.g., "ipc:///tmp/your_socket")
    zmq_connect(subscriber, config.address.c_str());

    // Subscribe topics
    std::vector<std::string> topics = {"rb", "MA"};
    for (auto&& topic : topics) {
        zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, topic.data(), topic.length());
    }

    TickData tick{};
    size_t count = 0;
    while (true) {
        zmq_recv(subscriber, &tick, sizeof(TickData), 0);
        ++count;
        auto tp = std::chrono::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds{tick.stamp}};
        std::println("recv: {},{},{}, count={}", tick.symbol, tp, tick.last, count);
    }

    // Cleanup (will not be reached in this infinite loop)
    zmq_close(subscriber);
    zmq_ctx_term(context);
}