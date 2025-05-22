#pragma once

#include <sys/syscall.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <format>
#include <optional>
#include <print>
#include <spanstream>
#include <string_view>

#include "ThostFtdcUserApiStruct.h"
#include "quote_type.h"
#include "string_parser.hpp"

template <typename Clock, typename Duration>
auto floor_to_interval(std::chrono::time_point<Clock, Duration> tp, Duration interval) {
    auto since = tp.time_since_epoch();
    auto rem = since % interval;
    return std::chrono::time_point<Clock, Duration>(since - rem);
}

inline auto str2tp(std::string_view dt_str, std::string_view fmt) {
    std::ispanstream in{dt_str};
    std::chrono::sys_seconds tp;
    std::chrono::from_stream(in, fmt.data(), tp);
    return tp;
}

inline void check_zoned_time(CThostFtdcDepthMarketDataField const* pDepthMarketData, int interval_seconds) {
    auto dt_str = std::format("{} {}", pDepthMarketData->ActionDay, pDepthMarketData->UpdateTime);
    std::chrono::local_seconds tp;
    std::istringstream in{dt_str};
    std::chrono::from_stream(in, "%Y%m%d %H:%M:%S", tp);

    auto t3 = floor_to_interval(tp, std::chrono::seconds{interval_seconds});

    std::println("t3={}, count={}", t3, t3.time_since_epoch().count());

    std::chrono::zoned_time zt{std::chrono::current_zone(), tp};
    std::println("zt={}", zt);

    auto utc_tp = zt.get_sys_time();
    std::println("utc_tp={}", utc_tp);

    auto tp_ms = utc_tp + std::chrono::milliseconds(pDepthMarketData->UpdateMillisec);
    std::println("tp_ms={}, count={}", tp_ms, tp_ms.time_since_epoch().count());

    auto tpn = std::chrono::system_clock::now();
    std::println("now={}", tpn);
    auto ztn = std::chrono::zoned_time{std::chrono::current_zone(), tpn};
    std::println("now zoned={}", ztn);
}

struct BarGenerator {
    BarGenerator(int interval) : _interval(interval) {}

    BarData process_tick(CThostFtdcDepthMarketDataField const* pDepthMarketData) {
        // TradingDay ? ActionDay ?
        auto dt_str = std::format("{} {}", pDepthMarketData->ActionDay, pDepthMarketData->UpdateTime);
        auto tp_last = str2tp(dt_str, "%Y%m%d %H:%M:%S");
        auto tp_start = floor_to_interval(tp_last, std::chrono::seconds{_interval});

        auto stamp_last = (tp_last + std::chrono::milliseconds(pDepthMarketData->UpdateMillisec)).time_since_epoch().count();
        auto stamp_start = std::chrono::duration_cast<std::chrono::milliseconds>(tp_start.time_since_epoch()).count();
        if (!_current) {
            // first tick
            _last_bar_vol = pDepthMarketData->Volume;
            _last_bar_amt = pDepthMarketData->Turnover;

            _current = BarData{};
            strncpy(_current->symbol, pDepthMarketData->InstrumentID, 31);
            _current->stamp_start = stamp_start;
            _current->stamp_last = stamp_last;
            _current->open = pDepthMarketData->LastPrice,
            _current->high = pDepthMarketData->LastPrice,
            _current->low = pDepthMarketData->LastPrice,
            _current->close = pDepthMarketData->LastPrice,
            _current->volume = 0;
            _current->amount = 0;
            _current->oi = pDepthMarketData->OpenInterest;
            _current->adj = 1.0;
            return _current.value();
        }

        if (stamp_start != _current->stamp_start) {
            // _current is closed bar
            _last_bar_vol += _current->volume;
            _last_bar_amt += _current->amount;
            // start new bar
            _current = BarData{};
            strncpy(_current->symbol, pDepthMarketData->InstrumentID, 31);
            _current->stamp_start = stamp_start;
            _current->stamp_last = stamp_last;
            _current->open = pDepthMarketData->LastPrice,
            _current->high = pDepthMarketData->LastPrice,
            _current->low = pDepthMarketData->LastPrice,
            _current->close = pDepthMarketData->LastPrice,
            _current->volume = 0;
            _current->amount = 0;
            _current->oi = pDepthMarketData->OpenInterest;
            _current->adj = 1.0;
            return _current.value();
        }

        // unclosed bar, still within the same bar
        _current->stamp_last = stamp_last;
        _current->high = std::max(_current->high, pDepthMarketData->LastPrice);
        _current->low = std::min(_current->low, pDepthMarketData->LastPrice);
        _current->close = pDepthMarketData->LastPrice;
        _current->volume = pDepthMarketData->Volume - _last_bar_vol;
        _current->amount = pDepthMarketData->Turnover - _last_bar_amt;
        _current->oi = pDepthMarketData->OpenInterest;
        return _current.value();
    }

   private:
    int _interval;
    std::optional<BarData> _current{};
    double _last_bar_vol{};
    double _last_bar_amt{};
};

struct TimeStampCalculator {
    int64_t compute(const char* action_day, const char* update_time, int millisec) {
        auto day = sv2int(action_day).value_or(0);
        if (day != _cached_day) {
            _cached_day = day;
            auto y = (action_day[0] - '0') * 1000 + (action_day[1] - '0') * 100 + (action_day[2] - '0') * 10 + (action_day[3] - '0');
            uint32_t m = (action_day[4] - '0') * 10 + (action_day[5] - '0');
            uint32_t d = (action_day[6] - '0') * 10 + (action_day[7] - '0');
            std::chrono::year_month_day ymd{std::chrono::year{y}, std::chrono::month{m}, std::chrono::day{d}};
            _cached_day_epoch = std::chrono::sys_days{ymd};
        }
        int h = (update_time[0] - '0') * 10 + (update_time[1] - '0');
        int mi = (update_time[3] - '0') * 10 + (update_time[4] - '0');
        int s = (update_time[6] - '0') * 10 + (update_time[7] - '0');
        auto tp = _cached_day_epoch + std::chrono::hours{h} + std::chrono::minutes{mi} + std::chrono::seconds{s} + std::chrono::milliseconds{millisec};
        return tp.time_since_epoch().count();
    }

   private:
    std::chrono::sys_days _cached_day_epoch{};
    int _cached_day{0};
};