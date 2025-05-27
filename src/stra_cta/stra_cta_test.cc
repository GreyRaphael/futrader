#include <print>

#include "aberration.hpp"
#include "i_strategy.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

void testConcept(StrategyConcept auto& strategy) {
    std::println("Strategy name: {}", strategy.name());
    TickData tick;  // Assuming TickData is defined elsewhere
    strategy.onTick(tick);
    std::println("Strategy executed successfully.");
}

TEST_CASE("aberration") {
    Aberration stg;
    testConcept(stg);
}