#include "duck_loader.h"

#include <duckdb.h>

#include <cassert>
#include <filesystem>
#include <format>
#include <memory>
#include <print>

#include "config_parser.h"

struct HistoryTickLoader::Impl {
    DuckdbConfig cfg;
    duckdb_database db;
    duckdb_connection conn;
    duckdb_result result;
};

HistoryTickLoader::HistoryTickLoader(std::string_view cfg_filename) : _pimpl(std::make_unique<Impl>()) {
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

bool HistoryTickLoader::query(std::string_view symbol, int64_t dt_start, int64_t dt_end) {
    auto sql = std::format("SELECT * FROM tick WHERE symbol='{}' AND dt_start>={} AND dt_end<={}", symbol, dt_start, dt_end);
    if (duckdb_query(_pimpl->conn, sql.data(), &_pimpl->result) != DuckDBSuccess) {
        duckdb_destroy_result(&_pimpl->result);
        return false;
    }

    auto rows = duckdb_row_count(&_pimpl->result);
    auto cols = duckdb_column_count(&_pimpl->result);
    for (auto r = 0; r < rows; r++) {
        auto id = duckdb_value_int64(&_pimpl->result, 0, r);
        auto age = duckdb_value_double(&_pimpl->result, 1, r);
        auto name = duckdb_value_string(&_pimpl->result, 2, r);
        auto s = std::string_view{name.data, name.size};
        std::println("id={},age={},name={}", id, age, s);
        duckdb_free(name.data);
    }
    return true;
}