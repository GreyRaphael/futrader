#pragma once

#include "../datatypes.h"

struct Aberation {
    const char* name();
    void on_bar(Bar const& bar);

   private:
    int base_length{40};
};