#include <filesystem>
#include <string_view>

#include "struct_parser.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <bt_md.hpp>

TEST_CASE("duckdb") {
    std::string_view cfg_filename{"bt.toml"};
    REQUIRE(std::filesystem::exists(cfg_filename));

    BactestMdClient bt_cli{cfg_filename, [](TickData const& tick) {
                               print_struct(&tick);
                           }};
    bt_cli.subscribe({"MA505", "rb2505"});
    bt_cli.start();
}