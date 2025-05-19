#pragma once

#include <cstdint>
#include <memory>
#include <spsc.hpp>
#include <string_view>

#include "quotetype.h"

using TickDataChannel = lockfree::SPSC<TickData, 1024>;
using TickDataChannelPtr = std::shared_ptr<TickDataChannel>;

struct HistoryTickLoader {
    HistoryTickLoader(std::string_view cfg_filename, TickDataChannelPtr channel_ptr);
    ~HistoryTickLoader();

   private:
    bool query(std::string_view symbol, int64_t dt_start, int64_t dt_end);

    struct Impl;
    std::unique_ptr<Impl> _pimpl{};

    TickDataChannelPtr _channel_ptr;
};