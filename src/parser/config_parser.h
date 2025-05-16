#pragma once
#include <string>

struct CtpConfig {
    std::string Name{};
    std::string UserProductInfo{};
    std::string BrokerID{};
    std::string AppID{};
    std::string AuthCode{};
    std::string TdInterface{};
    std::string TdFront{};
    std::string MdInterface{};
    std::string MdFront{};
    std::string UserID{};
    std::string Password{};

    static CtpConfig read_config(std::string_view filename);
};

struct BtConfig {
    std::string dt_start{};
    std::string dt_end{};
    std::string interval{};

    static BtConfig read_config(std::string_view filename);
};