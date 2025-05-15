#include <string_view>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>

#include "duck_loader.h"

TEST_CASE("openctp") {
    std::string_view db_path{"test.db"};
    REQUIRE(std::filesystem::exists(db_path));
}
