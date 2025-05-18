#pragma once

#include <cstdint>

struct BarData {
    int64_t stamp_start;
    int64_t stamp_last;
    double open;
    double high;
    double low;
    double close;
    double volume;
    double amount;
    double oi;
    double adj;
};
