#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

struct HistoryTickLoader {
    HistoryTickLoader(std::string_view cfg_filename);
    ~HistoryTickLoader();

   private:
    struct Impl;
    std::unique_ptr<Impl> _pimpl{};

    bool query(std::string_view symbol, int64_t dt_start, int64_t dt_end);
};