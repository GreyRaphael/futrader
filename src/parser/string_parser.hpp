#pragma once
#include <charconv>
#include <expected>
#include <string>

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