#include "abberation.h"

#include <algorithm>

#include "../rolling/mean.hpp"

const char* Abberation::name() {
    return "Abberation";
}

void Abberation::on_bar(Bar const& bar) {
    auto dyn_length = std::clamp(base_length, 20, 70);
    rolling::Mean meaner{};
}