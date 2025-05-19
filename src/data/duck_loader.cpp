#include "duck_loader.h"

#include <duckdb.h>

#include <cassert>
#include <cstring>
#include <filesystem>
#include <format>
#include <memory>
#include <print>

#include "config_parser.h"
#include "quotetype.h"

struct HistoryTickLoader::Impl {
    DuckdbConfig cfg{};
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

void HistoryTickLoader::Start() {
    auto sql = std::format("SELECT code, epoch_ms(dt), open FROM '{}' WHERE code IN ({}) AND dt>={} AND dt<={}", _pimpl->cfg.ParquetPath, "rb2507", _pimpl->cfg.DateStart, _pimpl->cfg.DateEnd);
    if (duckdb_query(_pimpl->conn, sql.data(), &_pimpl->result) != DuckDBSuccess) {
        duckdb_destroy_result(&_pimpl->result);
        return;
    }

    auto rows = duckdb_row_count(&_pimpl->result);
    auto cols = duckdb_column_count(&_pimpl->result);
    for (auto r = 0; r < rows; r++) {
        TickData tick{};
        auto code = duckdb_value_string(&_pimpl->result, 0, r);
        auto open = duckdb_value_double(&_pimpl->result, 2, r);
        strncpy(tick.symbol, code.data, code.size);
        tick.stamp = duckdb_value_int64(&_pimpl->result, 1, r);
        tick.open = duckdb_value_double(&_pimpl->result, 2, r);

        // free memeory of string
        duckdb_free(code.data);
    }
}