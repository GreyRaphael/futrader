#pragma once
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>

#include "i_order.h"
#include "i_quote.h"
#include "rolling.hpp"

struct Aberration {
    std::string name{"Aberration"};

    std::optional<Order> onTick(const TickData& tick) {
        auto mean = _meaner.update(tick.last);
        auto standard_deviation = _stder.update(tick.last);

        if (_position == 0) {
            if (tick.last > mean + 2 * standard_deviation) {
                // 价格偏离均值超过2个标准差，触发开仓
                memcpy(_order.symbol, tick.symbol, sizeof(SymbolType));
                _order.timestamp = tick.stamp;
                _order.volume = _lots;
                _order.direction = DirectionType::BUY;
                _order.offset = OffsetFlagType::OPEN;
                _position = 1;  // 开多头头寸
                return _order;
            }

            if (tick.last < mean - 2 * standard_deviation) {
                // 价格偏离均值超过2个标准差，触发开仓
                memcpy(_order.symbol, tick.symbol, sizeof(SymbolType));
                _order.timestamp = tick.stamp;
                _order.volume = _lots;
                _order.direction = DirectionType::SELL;
                _order.offset = OffsetFlagType::OPEN;
                _position = -1;  // 开空头头寸
                return _order;
            }
        }

        if (_position > 0) {
            // 持有多头头寸，检查是否需要平仓
            if (tick.last < mean) {
                memcpy(_order.symbol, tick.symbol, sizeof(SymbolType));
                _order.timestamp = tick.stamp;
                _order.volume = _lots;
                _order.direction = DirectionType::SELL;
                _order.offset = OffsetFlagType::CLOSE;
                _position = 0;  // 平仓后头寸归零
                return _order;
            }
        } else if (_position < 0) {
            // 持有空头头寸，检查是否需要平仓
            if (tick.last > mean) {
                memcpy(_order.symbol, tick.symbol, sizeof(SymbolType));
                _order.timestamp = tick.stamp;
                _order.volume = _lots;
                _order.direction = DirectionType::BUY;
                _order.offset = OffsetFlagType::CLOSE;
                _position = 0;  // 平仓后头寸归零
                return _order;
            }
        }

        return Order{
            "IC2506",
            1749001501000,
            1,
            DirectionType::BUY,
            OffsetFlagType::OPEN,
            "Aberration",
        };
    }

   private:
    rolling::Meaner _meaner{100};
    rolling::Stder _stder{100};
    int _position{0};
    uint32_t _lots{1};
    Order _order{"Aberration"};
};