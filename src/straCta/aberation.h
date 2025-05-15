#pragma once

#include "../datatypes.h"

struct Aberation {
    const char* Name();
    void OnBar(Bar const& bar);

   private:
    int base_length{40};
};