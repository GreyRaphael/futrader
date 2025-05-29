#pragma once
#include <cstdint>
#include <cstring>
#include <optional>

#include "i_order.h"
#include "i_quote.h"
#include "rolling.hpp"

struct Aberration {
    static const char* name() {
        return "Aberration";
    }

    std::optional<Order> onTick(const TickData& tick) {
        auto mean = _meaner.update(tick.last);
        auto standard_deviation = _stder.update(tick.last);
        Order order{};

        if (_position == 0) {
            if (tick.last > mean + 2 * standard_deviation) {
                // 价格偏离均值超过2个标准差，触发开仓
                memcpy(order.symbol, tick.symbol, sizeof(SymbolType));
                order.timestamp = tick.stamp;
                order.volume = _lots;
                order.direction = DirectionType::BUY;
                order.offset = OffsetFlagType::OPEN;
                _position = 1;  // 开多头头寸
                return order;
            }

            if (tick.last < mean - 2 * standard_deviation) {
                // 价格偏离均值超过2个标准差，触发开仓
                memcpy(order.symbol, tick.symbol, sizeof(SymbolType));
                order.timestamp = tick.stamp;
                order.volume = _lots;
                order.direction = DirectionType::SELL;
                order.offset = OffsetFlagType::OPEN;
                _position = -1;  // 开空头头寸
                return order;
            }
        }

        if (_position > 0) {
            // 持有多头头寸，检查是否需要平仓
            if (tick.last < mean) {
                memcpy(order.symbol, tick.symbol, sizeof(SymbolType));
                order.timestamp = tick.stamp;
                order.volume = _lots;
                order.direction = DirectionType::SELL;
                order.offset = OffsetFlagType::CLOSE;
                _position = 0;  // 平仓后头寸归零
                return order;
            }
        } else if (_position < 0) {
            // 持有空头头寸，检查是否需要平仓
            if (tick.last > mean) {
                memcpy(order.symbol, tick.symbol, sizeof(SymbolType));
                order.timestamp = tick.stamp;
                order.volume = _lots;
                order.direction = DirectionType::BUY;
                order.offset = OffsetFlagType::CLOSE;
                _position = 0;  // 平仓后头寸归零
                return order;
            }
        }

        return std::nullopt;
    }

   private:
    rolling::Meaner _meaner{100};
    rolling::Stder _stder{100};
    int _position{0};
    uint32_t _lots{1};
};