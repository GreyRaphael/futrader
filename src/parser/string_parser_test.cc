#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "string_parser.hpp"

#include <doctest/doctest.h>

#include <print>

TEST_CASE("testing the factorial function") {
    CHECK_EQ(sv2int("100"), 100);
    CHECK_EQ(sv2int("-123"), -123);
}