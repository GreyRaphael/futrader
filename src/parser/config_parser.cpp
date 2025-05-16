#include "config_parser.h"

#include <toml++/toml.h>

CtpConfig CtpConfig::read_config(std::string_view filename) {
    auto config = toml::parse_file(filename);
    auto mode = config["default"]["Mode"].value_or("openctp7x24");

    CtpConfig cfg{};
    cfg.Name = config[mode]["Name"].value_or("");
    cfg.UserProductInfo = config[mode]["UserProductInfo"].value_or("");
    cfg.BrokerID = config[mode]["BrokerID"].value_or("");
    cfg.AppID = config[mode]["AppID"].value_or("");
    cfg.AuthCode = config[mode]["AuthCode"].value_or("");
    cfg.TdInterface = config[mode]["TdInterface"].value_or("");
    cfg.TdFront = config[mode]["TdFront"].value_or("");
    cfg.MdInterface = config[mode]["MdInterface"].value_or("");
    cfg.MdFront = config[mode]["MdFront"].value_or("");
    cfg.UserID = config[mode]["UserID"].value_or("");
    cfg.Password = config[mode]["Password"].value_or("");
    return cfg;
}

BtConfig BtConfig::read_config(std::string_view filename) {
    // todo
    return BtConfig{};
}