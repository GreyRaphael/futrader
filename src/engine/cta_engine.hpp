#pragma once

#include <tuple>

#include "i_strategy.h"

template <typename MarketSource, CtaStgConcept... T>
struct CtaEngine {
    CtaEngine(MarketSource&& market_source, T&... cta_stgs)
        : _market_source(std::move(market_source)), _cta_stgs(&cta_stgs...) {
    }

    void run() {
    }
    void stop() {
    }

   private:
    MarketSource _market_source;
    std::tuple<T*...> _cta_stgs;
};
