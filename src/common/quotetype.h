#pragma once

#include <cstdint>

typedef char symbol_t[32];

struct BarData {
    symbol_t symbol;
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
