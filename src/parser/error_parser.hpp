#pragma once
#include <toml++/toml.h>

#include <concepts>
#include <map>
#include <print>
#include <string>

#include "string_parser.hpp"
#include "struct_parser.hpp"

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

// Define a concept that checks for a member named ErrorID
template <typename E>
concept HasErrorID = requires(E const &e) {
    { e.ErrorID } -> std::convertible_to<int>;
};

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
