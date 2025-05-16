#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "config_parser.h"

#include <doctest/doctest.h>

#include <filesystem>
#include <print>
#include <ylt/reflection/member_names.hpp>
#include <ylt/reflection/member_value.hpp>

// print any kinds of struct by reflection
template <typename T>
inline void print_struct(T const *ptr) noexcept {
    auto name = ylt::reflection::get_struct_name<T>();
    std::print("========> {}\n", name);
    ylt::reflection::for_each(*ptr, [](auto &field, auto name, auto index) {
        std::println("\t{}:{}", name, field);
    });
    std::println();
}

TEST_CASE("testing brokers.toml") {
    std::string_view filename{"brokers.toml"};
    REQUIRE(std::filesystem::exists(filename));
    auto config = CtpConfig::read_config(filename);
    print_struct(&config);
    std::filesystem::path lib_path{config.TdInterface};
    CHECK_EQ(lib_path.parent_path().string(), "ctp");
    CHECK_EQ(lib_path.filename().string(), "thosttraderapi_se.so");
}