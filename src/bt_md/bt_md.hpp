#pragma once

#include <duckdb.h>

#include <cassert>
#include <cstring>
#include <filesystem>
#include <print>
#include <ranges>
#include <string_view>
#include <vector>

#include "config_parser.h"
#include "quote_type.h"

template <typename CB>
struct BactestMdClient {
    BactestMdClient(std::string_view cfg_filename, CB callback) : _callback(callback) {
        // assert bt.toml exist
        assert(std::filesystem::exists(cfg_filename));
        _cfg = BtConfig::readConfig(cfg_filename);
        // open in-memory duckdb to read parquet files
        if (duckdb_open(nullptr, &_db) != DuckDBSuccess || duckdb_connect(_db, &_conn) != DuckDBSuccess) {
            throw std::runtime_error("Connection string is empty!");
        }
    }
    ~BactestMdClient() {
        duckdb_destroy_result(&_result);
        duckdb_disconnect(&_conn);
        duckdb_close(&_db);
    }

    void subscribe(std::vector<std::string> symbols);
    void start();

   private:
    CB _callback{};
    BtConfig _cfg{};
    std::string _symbols_holder;

    duckdb_database _db{};
    duckdb_connection _conn{};
    duckdb_result _result{};
};

template <typename CB>
void BactestMdClient<CB>::subscribe(std::vector<std::string> symbols) {
    // concat symbol with ','
    // mustn't use join_with(symbols, "','"); it join with `','\0`
    auto view = std::views::join_with(symbols, std::string_view{"','"});
    _symbols_holder.reserve(symbols.size() * 6 + 2);
    _symbols_holder.push_back('\'');
    // result.append("'"); // alternative
    _symbols_holder.append(view.begin(), view.end());
    _symbols_holder.push_back('\'');
}

template <typename CB>
void BactestMdClient<CB>::start() {
    // format sql
    auto sql = std::format(
        R"SQL(
        SELECT 
            code, 
            epoch_ms(dt), 
            last / 1e4,
            preclose / 1e4,
            presettle / 1e4,
            settle / 1e4
            preoi,
            oi,
            volume,
            amount / 1e4,
            ap1 / 1e4,
            ap2 / 1e4,
            ap3 / 1e4,
            ap4 / 1e4,
            ap5 / 1e4,
            bp1 / 1e4,
            bp2 / 1e4,
            bp3 / 1e4,
            bp4 / 1e4,
            bp5 / 1e4,
            av1,
            av2,
            av3,
            av4,
            av5,
            bv1,
            bv2,
            bv3,
            bv4,
            bv5,
        FROM '{}'
        WHERE code IN ({}) AND dt>='{}' AND dt<='{}'
        ORDER BY dt,volume;
        )SQL",
        _cfg.db_uri,
        _symbols_holder,
        _cfg.date_start,
        _cfg.date_end);

    std::println("sql={}", sql);
    if (duckdb_query(_conn, sql.data(), &_result) != DuckDBSuccess) {
        duckdb_destroy_result(&_result);
        std::println("query error, sql={}", sql);
        return;
    }

    // iter records
    auto rows = duckdb_row_count(&_result);
    TickData tick{};
    for (auto r = 0; r < rows; r++) {
        auto code = duckdb_value_string(&_result, 0, r);
        memcpy(tick.symbol, code.data, code.size);

        tick.stamp = duckdb_value_int64(&_result, 1, r);
        tick.last = duckdb_value_double(&_result, 2, r);
        tick.preclose = duckdb_value_double(&_result, 3, r);
        tick.presettle = duckdb_value_double(&_result, 4, r);
        tick.settle = duckdb_value_double(&_result, 5, r);
        tick.preoi = duckdb_value_uint64(&_result, 6, r);
        tick.oi = duckdb_value_uint64(&_result, 7, r);
        tick.volume = duckdb_value_uint64(&_result, 8, r);
        tick.amount = duckdb_value_double(&_result, 9, r);

        tick.ap1 = duckdb_value_double(&_result, 10, r);
        tick.ap2 = duckdb_value_double(&_result, 11, r);
        tick.ap3 = duckdb_value_double(&_result, 12, r);
        tick.ap4 = duckdb_value_double(&_result, 13, r);
        tick.ap5 = duckdb_value_double(&_result, 14, r);
        tick.bp1 = duckdb_value_double(&_result, 15, r);
        tick.bp2 = duckdb_value_double(&_result, 16, r);
        tick.bp3 = duckdb_value_double(&_result, 17, r);
        tick.bp4 = duckdb_value_double(&_result, 18, r);
        tick.bp5 = duckdb_value_double(&_result, 19, r);
        tick.av1 = duckdb_value_uint32(&_result, 20, r);
        tick.av2 = duckdb_value_uint32(&_result, 21, r);
        tick.av3 = duckdb_value_uint32(&_result, 22, r);
        tick.av4 = duckdb_value_uint32(&_result, 23, r);
        tick.av5 = duckdb_value_uint32(&_result, 24, r);
        tick.bv1 = duckdb_value_uint32(&_result, 25, r);
        tick.bv2 = duckdb_value_uint32(&_result, 26, r);
        tick.bv3 = duckdb_value_uint32(&_result, 27, r);
        tick.bv4 = duckdb_value_uint32(&_result, 28, r);
        tick.bv5 = duckdb_value_uint32(&_result, 29, r);

        _callback(tick);

        // free memeory of string
        duckdb_free(code.data);
    }
}