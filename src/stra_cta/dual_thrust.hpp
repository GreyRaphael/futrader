#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <optional>

#include "i_order.h"
#include "i_quote.h"

struct DualThrust {
    static const char* name() {
        return "DualThrust";
    }

    std::optional<Order> onTick(const TickData& tick) {
        // 初始化开盘价、最高价和最低价
        if (!_initialized) {
            _open = tick.last;
            _max = tick.last;
            _min = tick.last;
            _initialized = true;
        } else {
            _max = std::max(_max, tick.last);
            _min = std::min(_min, tick.last);
        }

        Order order{};

        // 计算买卖阈值
        double range = _max - _min;
        double buy_threshold = _open + _k1 * range;
        double sell_threshold = _open - _k2 * range;

        if (_position == 0) {
            if (tick.last >= buy_threshold) {
                // 价格突破买入阈值，触发开多头仓位
                memcpy(order.symbol, tick.symbol, sizeof(SymbolType));
                order.timestamp = tick.stamp;
                order.volume = _lots;
                order.direction = DirectionType::BUY;
                order.offset = OffsetFlagType::OPEN;
                _position = 1;
                return order;
            }
            if (tick.last <= sell_threshold) {
                // 价格突破卖出阈值，触发开空头仓位
                memcpy(order.symbol, tick.symbol, sizeof(SymbolType));
                order.timestamp = tick.stamp;
                order.volume = _lots;
                order.direction = DirectionType::SELL;
                order.offset = OffsetFlagType::OPEN;
                _position = -1;
                return order;
            }
        }

        if (_position > 0) {
            // 持有多头仓位，平仓条件：价格低于开盘价
            if (tick.last < _open) {
                memcpy(order.symbol, tick.symbol, sizeof(SymbolType));
                order.timestamp = tick.stamp;
                order.volume = _lots;
                order.direction = DirectionType::SELL;
                order.offset = OffsetFlagType::CLOSE;
                _position = 0;
                // 重置参数以便下一周期重新计算
                _initialized = false;
                return order;
            }
        } else if (_position < 0) {
            // 持有空头仓位，平仓条件：价格高于开盘价
            if (tick.last > _open) {
                memcpy(order.symbol, tick.symbol, sizeof(SymbolType));
                order.timestamp = tick.stamp;
                order.volume = _lots;
                order.direction = DirectionType::BUY;
                order.offset = OffsetFlagType::CLOSE;
                _position = 0;
                // 重置参数以便下一周期重新计算
                _initialized = false;
                return order;
            }
        }

        return std::nullopt;
    }

   private:
    int _position{0};  // 当前持仓：1 多头, -1 空头, 0 空仓
    uint32_t _lots{1};

    // DualThrust 参数
    double _k1{0.5};  // 可调买入系数
    double _k2{0.5};  // 可调卖出系数

    // 开盘价、最高价和最低价
    double _open{0.0};
    double _max{0.0};
    double _min{0.0};
    bool _initialized{false};
};