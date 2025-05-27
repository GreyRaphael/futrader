#pragma once
#include <concepts>
#include <optional>

#include "i_order.h"
#include "i_quote.h"

// Strategy concept requiring two methods: name and onTick
template <typename Strategy>
concept CtaStgConcept = requires(Strategy s, const TickData &t) {
    { s.name() } -> std::same_as<const char *>;
    { s.onTick(t) } -> std::same_as<std::optional<Order>>;
};