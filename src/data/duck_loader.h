#pragma once

#include <memory>
#include <spsc.hpp>
#include <string_view>
#include <vector>

#include "QuoteType.h"

using TickDataChannel = lockfree::SPSC<TickData, 1024>;
using TickDataChannelPtr = std::shared_ptr<TickDataChannel>;

struct HistoryTickLoader {
    HistoryTickLoader(std::string_view cfg_filename, TickDataChannelPtr channel_ptr);
    ~HistoryTickLoader();

    void Subscribe(std::vector<std::string> const &symbols);
    void Run();

   private:
    struct Impl;
    std::unique_ptr<Impl> _pimpl{};

    TickDataChannelPtr _channel_ptr;
};