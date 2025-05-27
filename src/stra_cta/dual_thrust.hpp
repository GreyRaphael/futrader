#pragma once
#include <algorithm>

#include "i_quote.h"
#include "i_order.h"
#include "rolling.hpp"

struct DualThrust {
    static const char* name() {
        return "DualThrust";
    }

    std::optional<Order> onTick(const TickData& tick) {
        auto dyn_length = std::clamp(_base_length, 20, 70);
        auto meaner = rolling::Meaner(dyn_length);
        // 原来的策略逻辑
        if (true /* 多头开仓 */) {
        } else {
            // 空头开仓
        }

        if (false /*多头平仓*/) {
        } else {
            // 空头平仓
        }
        return std::nullopt;
    }

   private:
    int _base_length = 30;  // 基础长度
};