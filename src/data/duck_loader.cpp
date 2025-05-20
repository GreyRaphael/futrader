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
    auto sql = std::format(
        R"SQL(
        SELECT 
            code, 
            epoch_ms(dt), 
            preclose,
            last,
            volume,
            amount,
            preoi,
            oi,
            ap0,
            ap1,
            ap2,
            ap3,
            ap4,
            bp0,
            bp1,
            bp2,
            bp3,
            bp4,
            av0,
            av1,
            av2,
            av3,
            av4,
            bv0,
            bv1,
            bv2,
            bv3,
            bv4,
            presettle,
            settle
        FROM '{}'
        WHERE code IN ({}) AND dt>='{}' AND dt<='{}';
        )SQL",
        _pimpl->cfg.ParquetPath,
        _pimpl->symbols_holder,
        _pimpl->cfg.DateStart,
        _pimpl->cfg.DateEnd);

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
        strncpy(tick.symbol, code.data, code.size);

        tick.stamp = duckdb_value_int64(&_pimpl->result, 1, r);
        tick.preclose = duckdb_value_uint64(&_pimpl->result, 2, r) / 1e4;
        tick.last = duckdb_value_uint64(&_pimpl->result, 3, r) / 1e4;
        tick.volume = duckdb_value_uint64(&_pimpl->result, 4, r);
        tick.amount = duckdb_value_uint64(&_pimpl->result, 5, r) / 1e4;
        tick.preoi = duckdb_value_uint64(&_pimpl->result, 6, r);
        tick.oi = duckdb_value_uint64(&_pimpl->result, 7, r);
        tick.ap1 = duckdb_value_uint64(&_pimpl->result, 8, r) / 1e4;
        tick.ap2 = duckdb_value_uint64(&_pimpl->result, 9, r) / 1e4;
        tick.ap3 = duckdb_value_uint64(&_pimpl->result, 10, r) / 1e4;
        tick.ap4 = duckdb_value_uint64(&_pimpl->result, 11, r) / 1e4;
        tick.ap5 = duckdb_value_uint64(&_pimpl->result, 12, r) / 1e4;
        tick.bp1 = duckdb_value_uint64(&_pimpl->result, 13, r) / 1e4;
        tick.bp2 = duckdb_value_uint64(&_pimpl->result, 14, r) / 1e4;
        tick.bp3 = duckdb_value_uint64(&_pimpl->result, 15, r) / 1e4;
        tick.bp4 = duckdb_value_uint64(&_pimpl->result, 16, r) / 1e4;
        tick.bp5 = duckdb_value_uint64(&_pimpl->result, 17, r) / 1e4;
        tick.av1 = duckdb_value_uint32(&_pimpl->result, 18, r);
        tick.av2 = duckdb_value_uint32(&_pimpl->result, 19, r);
        tick.av3 = duckdb_value_uint32(&_pimpl->result, 20, r);
        tick.av4 = duckdb_value_uint32(&_pimpl->result, 21, r);
        tick.av5 = duckdb_value_uint32(&_pimpl->result, 22, r);
        tick.bv1 = duckdb_value_uint32(&_pimpl->result, 23, r);
        tick.bv2 = duckdb_value_uint32(&_pimpl->result, 24, r);
        tick.bv3 = duckdb_value_uint32(&_pimpl->result, 25, r);
        tick.bv4 = duckdb_value_uint32(&_pimpl->result, 26, r);
        tick.bv5 = duckdb_value_uint32(&_pimpl->result, 27, r);
        tick.presettle = duckdb_value_uint64(&_pimpl->result, 28, r) / 1e4;
        tick.settle = duckdb_value_uint64(&_pimpl->result, 29, r) / 1e4;

        _channel_ptr->push(tick);
        // free memeory of string
        duckdb_free(code.data);
    }
}