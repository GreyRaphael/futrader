#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "config_parser.h"

#include <doctest/doctest.h>

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

TEST_CASE("testing the factorial function") {
    auto config = CtpConfig::read_config("openctp.toml");
    print_struct(&config);
}