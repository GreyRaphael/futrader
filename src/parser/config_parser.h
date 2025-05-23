#pragma once
#include <string>
#include <vector>

struct CtpConfig {
    std::string UserProductInfo{};
    std::string BrokerID{};
    std::string AppID{};
    std::string AuthCode{};
    std::string Interface{};
    std::string Front{};
    std::string UserID{};
    std::string Password{};

    static CtpConfig read_config(std::string_view filename, std::string_view user_type);
};

struct BtConfig {
    std::string db_uri{};
    std::string date_start{};
    std::string date_end{};

    static BtConfig readConfig(std::string_view filename);
};

struct NngConfig {
    std::string address;
    int poll_interval_ms;
    std::string broker_file;
    std::vector<std::string> symbols;

    static NngConfig readConfig(std::string_view filename);
};

struct DuckdbConfig {
    std::string ParquetPath;
    std::string DateStart;
    std::string DateEnd;

    static DuckdbConfig read_config(std::string_view filename);
};