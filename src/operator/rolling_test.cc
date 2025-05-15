#include <cmath>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "rolling.hpp"

TEST_CASE("testing the factorial function") {
    rolling::Sumer sumer{3};
    CHECK(std::isnan(sumer.update(10)));
    CHECK(std::isnan(sumer.update(20)));
    auto v = sumer.update(30);
    CHECK(!std::isnan(v));
    CHECK_EQ(v, 60);
}