#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "struct_parser.hpp"

#include <doctest/doctest.h>

struct MyStruct {
    int id;
    double score;
    std::string name;
};

TEST_CASE("testing the factorial function") {
    MyStruct obj{100, 99.5, "Tom"};
    print_struct(&obj);
}