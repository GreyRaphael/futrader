#pragma once
#include <print>
#include <ylt/reflection/member_names.hpp>
#include <ylt/reflection/member_value.hpp>

// print any kinds of struct by reflection
template <typename T>
inline void print_struct(T const *ptr) noexcept {
    auto name = ylt::reflection::get_struct_name<T>();
    std::print("========> {}, size={}\n\t", name, sizeof(T));
    ylt::reflection::for_each(*ptr, [](auto &field, auto name, auto index) {
        std::print("{}:{} | ", name, field);
    });
    std::println();
}