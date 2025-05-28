#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "engine_cta.hpp"
#include "stra_cta.h"

TEST_CASE("cta") {
    std::vector<CtaStrategy> cta_stgs;
    CtaZmqEngine engine{"zmq.toml", cta_stgs};
}