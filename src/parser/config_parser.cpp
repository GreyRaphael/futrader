#include "config_parser.h"

#include <toml++/toml.h>

#include <string_view>
#include <vector>

CtpConfig CtpConfig::read_config(std::string_view filename, std::string_view user_type) {
    auto config = toml::parse_file(filename);

    CtpConfig cfg{};
    cfg.UserProductInfo = config[user_type]["UserProductInfo"].value_or("");
    cfg.BrokerID = config[user_type]["BrokerID"].value_or("");
    cfg.AppID = config[user_type]["AppID"].value_or("");
    cfg.AuthCode = config[user_type]["AuthCode"].value_or("");
    cfg.Interface = config[user_type]["Interface"].value_or("");
    cfg.Front = config[user_type]["Front"].value_or("");
    cfg.UserID = config[user_type]["UserID"].value_or("");
    cfg.Password = config[user_type]["Password"].value_or("");
    return cfg;
}

BtConfig BtConfig::readConfig(std::string_view filename) {
    auto config = toml::parse_file(filename);

    BtConfig cfg{};
    cfg.db_uri = config["db_uri"].value_or("futures.parquet");
    cfg.date_start = config["date_start"].value_or("2025-01-01");
    cfg.date_end = config["date_end"].value_or("2025-02-01");
    return cfg;
}

NngConfig NngConfig::readConfig(std::string_view filename) {
    auto config = toml::parse_file(filename);

    NngConfig cfg{};
    cfg.address = config["Address"].value_or("ipc:///tmp/pubsub.ipc");
    cfg.broker_file = config["BrokerFile"].value_or("tts.toml");
    cfg.poll_interval_ms = config["PollIntervalMs"].value_or(0);
    if (auto arr = config["Symbols"].as_array()) {
        for (auto &&e : *arr) {
            cfg.symbols.push_back(e.value_or(""));
        }
    }
    return cfg;
}

DuckdbConfig DuckdbConfig::read_config(std::string_view filename) {
    auto config = toml::parse_file(filename);

    DuckdbConfig cfg{};
    cfg.ParquetPath = config["ParquetPath"].value_or("futures.parquet");
    cfg.DateStart = config["DateStart"].value_or("2025-01-01");
    cfg.DateEnd = config["DateEnd"].value_or("2025-02-01");
    return cfg;
}