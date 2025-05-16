#include "config_parser.h"

#include <toml++/toml.h>

#include <string_view>

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

BtConfig BtConfig::read_config(std::string_view filename) {
    // todo
    return BtConfig{};
}