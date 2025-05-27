#pragma once
#include <concepts>

#include "quote_type.h"

// Strategy concept requiring two methods: name and onTick
template <typename Strategy>
concept StrategyConcept = requires(Strategy s, const TickData &t) {
    { s.name() } -> std::convertible_to<const char *>;
    { s.onTick(t) } -> std::same_as<void>;
};