#pragma once
#include <toml++/toml.h>

#include <map>
#include <string>
#include <string_view>
#include <utils/parser.hpp>

namespace errconfig {

inline const std::map<int, std::string> errors = [] {
    auto tbl = toml::parse_file("errors.toml");
    std::map<int, std::string> errs{};
    for (auto& [k, v] : tbl) {
        auto k_expected = sv2int(k.str());
        if (k_expected) {
            // parse success
            errs.emplace(k_expected.value(), v.value_or(""));
        }
    }
    return errs;
}();

// 2) easy accessor
inline std::string_view get(int key) {
    if (errors.contains(key)) {
        return errors.at(key);
    } else {
        return "no exist";
    }
}
}  // namespace errconfig

namespace ctpconfig {

struct UsrConfig {
    std::string mode;
    std::string platform;
    std::string front_td;
    std::string front_md;
    std::string auth_id;
    std::string auth_code;
    std::string broker_id;
    std::string user_id;
    std::string password;
};

inline const UsrConfig cfg = [] {
    auto config = toml::parse_file("ctp.toml");
    std::string mode = config["default"]["mode"].value_or("openctp");
    std::string platform = config["default"]["platform"].value_or("lin64");
    std::string front_td = config[mode]["front_td"].value_or("");
    std::string front_md = config[mode]["front_md"].value_or("");
    std::string auth_id = config[mode]["auth_id"].value_or("");
    std::string auth_code = config[mode]["auth_code"].value_or("");
    std::string broker_id = config[mode]["broker_id"].value_or("");
    std::string user_id = config[mode]["user_id"].value_or("");
    std::string password = config[mode]["password"].value_or("");

    return UsrConfig{
        mode,
        platform,
        front_td,
        front_md,
        auth_id,
        auth_code,
        broker_id,
        user_id,
        password,
    };
}();

}  // namespace ctpconfig