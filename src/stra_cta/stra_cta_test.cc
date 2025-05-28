#include <print>
#include <variant>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "aberration.hpp"
#include "dual_thrust.hpp"
#include "stra_cta.h"

TEST_CASE("aberration") {
    Aberration stg;
}

TEST_CASE("dual_thrust") {
    DualThrust stg;
}

TEST_CASE("polymorphism") {
    std::vector<CtaStrategy> strategies;
    strategies.emplace_back(Aberration{});
    strategies.emplace_back(DualThrust{});

    for (const auto& strategy : strategies) {
        std::visit([](const auto& stg) { std::println("strategy name={}", stg.name()); }, strategy);
    }
}