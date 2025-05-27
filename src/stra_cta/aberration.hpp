#pragma once
#include <algorithm>

#include "quote_type.h"
#include "rolling.hpp"

struct Aberration {
    const char* name() {
        return "Aberration";
    }

    void onTick(const TickData& tick) {
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
    }

   private:
    int _base_length = 30;  // 基础长度
};