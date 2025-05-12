#pragma once

#include "../quote.h"

struct Abberation {
    const char* name();
    void on_bar(Bar const& bar);

   private:
    int base_length{40};
};