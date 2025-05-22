#pragma once

#include <cstdint>

typedef char symbol_t[32];

struct BarData {
    symbol_t symbol;
    int64_t stamp_start, stamp_last;
    double open, high, low, close;
    int64_t volume;
    double amount, oi;
    double adj;
};

struct TickData {
    symbol_t symbol;
    int64_t stamp;
    double open, high, low, last;
    double limit_up, limit_down;
    double close, preclose;
    double settle, presettle;
    double oi, preoi;
    int64_t volume;
    double amount, avgprice;
    double ap1, ap2, ap3, ap4, ap5;
    double bp1, bp2, bp3, bp4, bp5;
    int av1, av2, av3, av4, av5;
    int bv1, bv2, bv3, bv4, bv5;
    double adj;
};