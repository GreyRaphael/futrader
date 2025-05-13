#include "reader.h"

#include <duckdb.h>

#include <format>
#include <memory>
#include <print>

struct HistoryTickLoader::Impl {
    duckdb_database db;
    duckdb_connection conn;
    duckdb_result result;
};

HistoryTickLoader::HistoryTickLoader(const char* db_path) : pImpl(std::make_unique<Impl>()) {
    if (duckdb_open(db_path, &pImpl->db) != DuckDBSuccess || duckdb_connect(pImpl->db, &pImpl->conn) != DuckDBSuccess) {
        throw std::runtime_error("Connection string is empty!");
    }
}

HistoryTickLoader::~HistoryTickLoader() {
    duckdb_destroy_result(&pImpl->result);
    duckdb_disconnect(&pImpl->conn);
    duckdb_close(&pImpl->db);
}

bool HistoryTickLoader::query(std::string_view symbol, int64_t dt_start, int64_t dt_end) {
    auto sql = std::format("SELECT * FROM tick WHERE symbol='{}' AND dt_start>={} AND dt_end<={}", symbol, dt_start, dt_end);
    if (duckdb_query(pImpl->conn, sql.data(), &pImpl->result) != DuckDBSuccess) {
        duckdb_destroy_result(&pImpl->result);
        return false;
    }

    auto rows = duckdb_row_count(&pImpl->result);
    auto cols = duckdb_column_count(&pImpl->result);
    for (auto r = 0; r < rows; r++) {
        auto id = duckdb_value_int64(&pImpl->result, 0, r);
        auto age = duckdb_value_double(&pImpl->result, 1, r);
        auto name = duckdb_value_string(&pImpl->result, 2, r);
        auto s = std::string_view{name.data, name.size};
        std::println("id={},age={},name={}", id, age, s);
        duckdb_free(name.data);
    }
    return true;
}