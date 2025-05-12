#pragma once

#include <cstdint>

struct Bar {
    std::int64_t dt_start;
    std::int64_t dt_end;
    double open;
    double high;
    double low;
    double close;
    double volume;
    double amount;
    double oi;
};

struct Tick {
    std::int64_t dt;
    double open;
    double last;
    double volume;
    double amount;
    double oi;
};