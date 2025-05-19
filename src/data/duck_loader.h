#pragma once

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
    void Start();

    struct Impl;
    std::unique_ptr<Impl> _pimpl{};

    TickDataChannelPtr _channel_ptr;
};