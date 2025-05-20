#include <string_view>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include <print>
#include <thread>

#include "duck_loader.h"

TEST_CASE("duckdb") {
    std::string_view cfg_filename{"duckdb.toml"};
    REQUIRE(std::filesystem::exists(cfg_filename));

    auto channel_ptr = std::make_shared<TickDataChannel>();
    HistoryTickLoader loader{cfg_filename, channel_ptr};
    loader.Subscribe({"MA509", "rb2507"});
    loader.Run();

    while (true) {
        auto value = channel_ptr->pop();
        if (!value) {
            std::println("empty, cannot pop");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }
        // print_struct(&value);
        std::println("{},{},{}", value->stamp, value->symbol, value->last);
    }
}
