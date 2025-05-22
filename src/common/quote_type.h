#pragma once

#include <cstdint>

typedef char symbol_t[32];

struct BarData {
    symbol_t symbol;
    int64_t stamp_start, stamp_last;
    double open, high, low, close;
    int64_t volume, oi;
    double amount;
    double adj;
};

struct TickData {
    symbol_t symbol;
    int64_t stamp;
    double open, high, low, last, close, preclose;
    double limit_up, limit_down;
    double settle, presettle;
    int64_t volume;
    double oi, preoi;
    double amount;
    double avgprice;
    double ap1, ap2, ap3, ap4, ap5;
    double bp1, bp2, bp3, bp4, bp5;
    int av1, av2, av3, av4, av5;
    int bv1, bv2, bv3, bv4, bv5;
    double adj;
};