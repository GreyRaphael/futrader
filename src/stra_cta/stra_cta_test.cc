#include <print>

#include "aberration.hpp"
#include "dual_thrust.hpp"
#include "i_strategy.h"
#include "i_quote.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

void testConcept(CtaStgConcept auto& strategy) {
    std::println("Strategy name: {}", strategy.name());
    TickData tick;  // Assuming TickData is defined elsewhere
    strategy.onTick(tick);
    std::println("Strategy executed successfully.");
}

TEST_CASE("aberration") {
    Aberration stg;
    testConcept(stg);
}

TEST_CASE("dual_thrust") {
    DualThrust stg;
    testConcept(stg);
}