#pragma once
#include <toml++/toml.h>

#include <charconv>
#include <expected>
#include <string>

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

inline std::map<int, std::string> load_errors(std::string_view filename) {
    auto tbl = toml::parse_file(filename);
    std::map<int, std::string> errs{};
    for (auto &[k, v] : tbl) {
        auto k_expected = sv2int(k.str());
        if (k_expected) {
            // parse success
            errs.emplace(k_expected.value(), v.value_or(""));
        }
    }
    return errs;
}