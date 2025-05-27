#pragma once

#include <cstdint>

#include "i_quote.h"

enum class DirectionType : uint8_t {
    NONE,
    BUY,
    SELL,
};

enum class OffsetFlagType : uint8_t {
    NONE,
    OPEN,
    CLOSE,
};

struct Order {
    SymbolType symbol;        // Symbol of the order
    int64_t timestamp;        // Order timestamp in milliseconds
    uint32_t volume;          // Volume of the order
    DirectionType direction;  // Direction of the order (BUY/SELL)
    OffsetFlagType offset;    // Offset flag (OPEN/CLOSE)
};