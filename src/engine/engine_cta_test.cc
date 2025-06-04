#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "engine_cta.hpp"

#include <doctest/doctest.h>

#include "aberration.hpp"
#include "stra_cta.h"

TEST_CASE("cta") {
    CtaEngine engine{"zmq.toml", 4};
    Aberration abb{};
    engine.addStrategy("IC2506", abb);
    engine.start();
}