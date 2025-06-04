#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "engine_cta.hpp"

#include <doctest/doctest.h>

#include "aberration.hpp"
#include "stra_cta.h"

TEST_CASE("bt") {
    CtaEngine engine{"ipc://@hq", 4};
    Aberration abb{};
    engine.addStrategy("rb2505", abb);
    // engine.addStrategy("MA5055", abb);
    engine.start();
}