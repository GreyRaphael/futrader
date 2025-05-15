#include "config.h"

#include <toml++/toml.h>

CtpConfig CtpConfig::read_config(std::string_view filename) {
    auto config = toml::parse_file(filename);
    std::string lib_dir = config["default"]["lib_dir"].value_or("openctp");
    std::string platform = config["default"]["platform"].value_or("lin64");
    std::string front_td = config["user"]["front_td"].value_or("");
    std::string front_md = config["user"]["front_md"].value_or("");
    std::string auth_id = config["user"]["auth_id"].value_or("");
    std::string auth_code = config["user"]["auth_code"].value_or("");
    std::string broker_id = config["user"]["broker_id"].value_or("");
    std::string user_id = config["user"]["user_id"].value_or("");
    std::string password = config["user"]["password"].value_or("");

    return CtpConfig{
        lib_dir,
        platform,
        front_td,
        front_md,
        auth_id,
        auth_code,
        broker_id,
        user_id,
        password,
    };
}

BtConfig BtConfig::read_config(std::string_view filename) {
    // todo
    return BtConfig{};
}