#include "aberation.h"

#include <algorithm>

#include "../operators/rolling.hpp"

const char* Aberation::Name() {
    return "Abberation";
}

void Aberation::OnBar(Bar const& bar) {
    auto dyn_length = std::clamp(base_length, 20, 70);
    auto meaner = rolling::Meaner(dyn_length);
}