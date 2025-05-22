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

TEST_CASE("testing ctp.toml") {
    std::string_view filename{"ctp.toml"};
    REQUIRE(std::filesystem::exists(filename));

    auto td_config = CtpConfig::read_config(filename, "td");
    print_struct(&td_config);
    std::filesystem::path td_libpath{td_config.Interface};
    CHECK_EQ(td_libpath.parent_path().string(), "ctp");
    CHECK_EQ(td_libpath.filename().string(), "thosttraderapi_se.so");

    auto md_config = CtpConfig::read_config(filename, "md");
    print_struct(&md_config);
    std::filesystem::path md_libpath{md_config.Interface};
    CHECK_EQ(md_libpath.parent_path().string(), "ctp");
    CHECK_EQ(md_libpath.filename().string(), "thostmduserapi_se.so");
}

TEST_CASE("testing tts.toml") {
    std::string_view filename{"tts.toml"};
    REQUIRE(std::filesystem::exists(filename));

    auto td_config = CtpConfig::read_config(filename, "td");
    print_struct(&td_config);
    std::filesystem::path td_libpath{td_config.Interface};
    CHECK_EQ(td_libpath.parent_path().string(), "tts");
    CHECK_EQ(td_libpath.filename().string(), "thosttraderapi_se.so");

    auto md_config = CtpConfig::read_config(filename, "md");
    print_struct(&md_config);
    std::filesystem::path md_libpath{md_config.Interface};
    CHECK_EQ(md_libpath.parent_path().string(), "tts");
    CHECK_EQ(md_libpath.filename().string(), "thostmduserapi_se.so");
}

TEST_CASE("testing nng.toml") {
    std::string_view filename{"nng.toml"};
    REQUIRE(std::filesystem::exists(filename));

    auto nng_config = NngConfig::read_config(filename);
    CHECK_EQ(nng_config.Address, "ipc:///tmp/pubsub.ipc");
    CHECK_EQ(nng_config.BrokerFile, "tts.toml");
    CHECK_EQ(nng_config.Symbols[0], "MA509");
    CHECK_EQ(nng_config.Symbols[1], "rb2507");
}