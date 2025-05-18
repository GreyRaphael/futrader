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
    std::string dt_start{};
    std::string dt_end{};
    std::string interval{};

    static BtConfig read_config(std::string_view filename);
};

struct NngConfig {
    std::string Address;
    int PollIntervalMs;
    std::string BrokerFile;
    std::vector<std::string> Symbols;

    static NngConfig read_config(std::string_view filename);
};