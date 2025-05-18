#include <cstring>

#include "ThostFtdcUserApiStruct.h"
#include "struct_parser.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "tick_parser.hpp"

TEST_CASE("test chrono") {
    CThostFtdcDepthMarketDataField md{};
    strcpy(md.ActionDay, "20250518");
    strcpy(md.UpdateTime, "21:19:34");
    md.UpdateMillisec = 500;
    check_zoned_time(&md, 180);
}

TEST_CASE("test tick2bar") {
    // bar interval is 60s(1min)
    BarGenerator bg{60};

    CThostFtdcDepthMarketDataField md{};
    strcpy(md.ActionDay, "20250518");
    strcpy(md.UpdateTime, "21:19:34");
    md.UpdateMillisec = 500;
    md.LastPrice = 10;
    md.Volume = 2000;
    md.Turnover = 22000;
    md.OpenInterest=100;
    auto bar = bg.process_tick(&md);
    print_struct(&bar);

    strcpy(md.ActionDay, "20250518");
    strcpy(md.UpdateTime, "21:19:59");
    md.UpdateMillisec = 500;
    md.LastPrice = 11;
    md.Volume = 4000;
    md.Turnover = 42000;
    md.OpenInterest=200;
    bar = bg.process_tick(&md);
    print_struct(&bar);

    strcpy(md.ActionDay, "20250518");
    strcpy(md.UpdateTime, "21:20:14");
    md.LastPrice = 12;
    md.Volume = 6500;
    md.Turnover = 63000;
    md.OpenInterest=300;
    bar = bg.process_tick(&md);
    print_struct(&bar);
}