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

struct TickData {
    symbol_t symbol;
    int64_t stamp;
    double open;
    double high;
    double low;
    double last;
    double close;
    double settle;
    double limit_up;
    double limit_down;
    double preclose;
    double presettle;
    double volume;
    double amount;
    double oi;
    double preoi;
    double avgprice;
    double adj;
    double ap1;
    double ap2;
    double ap3;
    double ap4;
    double ap5;
    double bp1;
    double bp2;
    double bp3;
    double bp4;
    double bp5;
    int av1;
    int av2;
    int av3;
    int av4;
    int av5;
    int bv1;
    int bv2;
    int bv3;
    int bv4;
    int bv5;
};