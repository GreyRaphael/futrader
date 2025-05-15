#pragma once
#include <string>

struct CtpConfig {
    std::string lib_dir{};
    std::string platform{};
    std::string front_td{};
    std::string front_md{};
    std::string auth_id{};
    std::string auth_code{};
    std::string broker_id{};
    std::string user_id{};
    std::string password{};

    static CtpConfig read_config(std::string_view filename);
};

struct BtConfig {
    std::string dt_start{};
    std::string dt_end{};
    std::string interval{};

    static BtConfig read_config(std::string_view filename);
};