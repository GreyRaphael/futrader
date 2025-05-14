#include "abberation.h"

#include <algorithm>
#include <memory>

#include "../operators/rolling.hpp"

const char* Abberation::name() {
    return "Abberation";
}

void Abberation::on_bar(Bar const& bar) {
    auto dyn_length = std::clamp(base_length, 20, 70);
    auto meaner = rolling::Meaner(dyn_length);
}