#include "duck_loader.h"

#include <duckdb.h>

#include <cassert>
#include <cstring>
#include <filesystem>
#include <format>
#include <memory>
#include <print>
#include <ranges>
#include <string>
#include <utility>

#include "config_parser.h"
#include "quotetype.h"

struct HistoryTickLoader::Impl {
    DuckdbConfig cfg{};
    std::string symbols_holder{};
    duckdb_database db;
    duckdb_connection conn;
    duckdb_result result;
};

HistoryTickLoader::HistoryTickLoader(std::string_view cfg_filename, TickDataChannelPtr channel_ptr)
    : _pimpl(std::make_unique<Impl>()), _channel_ptr(channel_ptr) {
    // assert duckdb.toml exist
    assert(std::filesystem::exists(cfg_filename));
    // read toml config
    _pimpl->cfg = DuckdbConfig::read_config(cfg_filename);
    // open inmemory duckdb to read parquet files
    if (duckdb_open(nullptr, &_pimpl->db) != DuckDBSuccess || duckdb_connect(_pimpl->db, &_pimpl->conn) != DuckDBSuccess) {
        throw std::runtime_error("Connection string is empty!");
    }
}

HistoryTickLoader::~HistoryTickLoader() {
    duckdb_destroy_result(&_pimpl->result);
    duckdb_disconnect(&_pimpl->conn);
    duckdb_close(&_pimpl->db);
}

void HistoryTickLoader::Subscribe(std::vector<std::string> const &symbols) {
    // mustn't use join_with(symbols, "','"); it join with `','\0`
    auto view = std::views::join_with(symbols, std::string_view{"','"});
    std::string result;
    result.reserve(symbols.size() * 6 + 2);
    result.push_back('\'');
    // result.append("'"); // alternative
    result.append(view.begin(), view.end());
    result.push_back('\'');
    _pimpl->symbols_holder = std::move(result);
}

void HistoryTickLoader::Run() {
    auto sql = std::format("SELECT code, epoch_ms(dt), last FROM '{}' WHERE code IN ({}) AND dt>='{}' AND dt<='{}';", _pimpl->cfg.ParquetPath, _pimpl->symbols_holder, _pimpl->cfg.DateStart, _pimpl->cfg.DateEnd);
    std::println("sql={}", sql);
    if (duckdb_query(_pimpl->conn, sql.data(), &_pimpl->result) != DuckDBSuccess) {
        duckdb_destroy_result(&_pimpl->result);
        std::println("query error");
        return;
    }

    auto rows = duckdb_row_count(&_pimpl->result);
    auto cols = duckdb_column_count(&_pimpl->result);
    for (auto r = 0; r < rows; r++) {
        TickData tick{};
        auto code = duckdb_value_string(&_pimpl->result, 0, r);
        auto last = duckdb_value_double(&_pimpl->result, 2, r);
        strncpy(tick.symbol, code.data, code.size);
        tick.stamp = duckdb_value_int64(&_pimpl->result, 1, r);
        tick.last = duckdb_value_double(&_pimpl->result, 2, r);

        _channel_ptr->push(tick);
        // free memeory of string
        duckdb_free(code.data);
    }
}