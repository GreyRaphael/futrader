#pragma once

#include <cstdint>

typedef char symbol_t[32];
typedef int volume_array_t[5];
typedef double price_array_t[5];

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
    double amouont;
    double oi;
    double preoi;
    double avgprice;
    double adj;
    volume_array_t ask_volumes;
    volume_array_t bid_volumes;
    price_array_t ask_prices;
    price_array_t bid_prices;
};