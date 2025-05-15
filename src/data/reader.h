#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

struct HistoryTickLoader {
    HistoryTickLoader(const char* db_path);
    ~HistoryTickLoader();

   private:
    struct Impl;
    std::unique_ptr<Impl> pImpl{};

    bool query(std::string_view symbol, int64_t dt_start, int64_t dt_end);
};