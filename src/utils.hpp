#pragma once
#include <toml++/toml.h>

#include <charconv>
#include <concepts>
#include <expected>
#include <print>
#include <string>
#include <ylt/reflection/member_names.hpp>
#include <ylt/reflection/member_value.hpp>

// parse string to int
inline std::expected<int, std::string> sv2int(std::string_view input) {
    int value{};
    auto [ptr, ec] = std::from_chars(input.begin(), input.end(), value);

    if (ec == std::errc())
        return value;

    if (ec == std::errc::invalid_argument)
        return std::unexpected("Invalid number format");
    else if (ec == std::errc::result_out_of_range)
        return std::unexpected("Number out of range");

    return std::unexpected("Unknown conversion error");
}

// print any kinds of struct by reflection
template <typename T>
inline void print_struct(T const *ptr) noexcept {
    auto name = ylt::reflection::get_struct_name<T>();
    std::print("========> {}\n\t", name);
    ylt::reflection::for_each(*ptr, [](auto &field, auto name, auto index) {
        std::print("{}:{} | ", name, field);
    });
    std::println();
}

// Define a concept that checks for a member named ErrorID
template <typename E>
concept HasErrorID = requires(E const &e) {
    { e.ErrorID } -> std::convertible_to<int>;
};

// error config
namespace errconfig {

inline const std::map<int, std::string> errors = [] {
    auto tbl = toml::parse_file("errors.toml");
    std::map<int, std::string> errs{};
    for (auto &[k, v] : tbl) {
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

inline const std::map<int, std::string> discon_errors{
    {0x1001, "网络读失败"},
    {0x1002, "网络写失败"},
    {0x2001, "接收心跳超时"},
    {0x2002, "发送心跳失败"},
    {0x2003, "收到错误报文"},
};
}  // namespace errconfig

// handle any kinds of reponse
template <typename T, typename E>
    requires HasErrorID<E>
inline void handle_resp(T const *ptr, E const *pRspInfo) noexcept {
    auto name = ylt::reflection::get_struct_name<T>();
    if (pRspInfo) {
        std::println("{}, {}", errconfig::get(pRspInfo->ErrorID), name);
    }

    if (ptr) {
        print_struct(ptr);
    } else {
        std::println("nothing in {}", name);
    }
}

// CtpConfig struct
struct CtpConfig {
    std::string lib_dir;
    std::string platform;
    std::string front_td;
    std::string front_md;
    std::string auth_id;
    std::string auth_code;
    std::string broker_id;
    std::string user_id;
    std::string password;
};

// read *.toml file to CtpConfig
inline CtpConfig read_config(std::string_view file_name) {
    auto config = toml::parse_file(file_name);
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