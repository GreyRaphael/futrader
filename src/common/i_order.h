#pragma once

#include <cstdint>
#include <format>

#include "i_quote.h"

typedef char StrategyType[32];

enum class DirectionType : uint8_t {
    BUY,
    SELL,
};

enum class OffsetFlagType : uint8_t {
    OPEN,
    CLOSE,
};

struct Order {
    StrategyType stg_name;    // Name of the strategy that generated the order
    SymbolType symbol;        // Symbol of the order
    int64_t timestamp;        // Order timestamp in milliseconds
    uint32_t volume;          // Volume of the order
    DirectionType direction;  // Direction of the order (BUY/SELL)
    OffsetFlagType offset;    // Offset flag (OPEN/CLOSE)
};

// The folloiwng is for print_struct
namespace std {
//  A “catch‐all” formatter for any enum where the underlying type is uint8_t.
//  It simply casts the enum to uint8_t and reuses std::formatter<uint8_t>.
template <typename E>
    requires(std::is_enum_v<E> && std::is_same_v<std::underlying_type_t<E>, uint8_t>)
struct formatter<E> : formatter<uint8_t> {
    auto format(E e, format_context &ctx) const {
        return formatter<uint8_t>::format(static_cast<uint8_t>(e), ctx);
    }
};
}  // namespace std

// template <>
// struct std::formatter<DirectionType> : std::formatter<std::string_view> {
//     auto format(DirectionType dir, format_context &ctx) const {
//         std::string_view name = "UNKNOWN";
//         switch (dir) {
//             case DirectionType::BUY:
//                 name = "BUY";
//                 break;
//             case DirectionType::SELL:
//                 name = "SELL";
//                 break;
//         }
//         return std::formatter<std::string_view>::format(name, ctx);
//     }
// };

// template <>
// struct std::formatter<OffsetFlagType> : std::formatter<std::string_view> {
//     auto format(OffsetFlagType flag, format_context &ctx) const {
//         std::string_view name = "UNKNOWN";
//         switch (flag) {
//             case OffsetFlagType::OPEN:
//                 name = "OPEN";
//                 break;
//             case OffsetFlagType::CLOSE:
//                 name = "CLOSE";
//                 break;
//         }
//         return std::formatter<std::string_view>::format(name, ctx);
//     }
// };
