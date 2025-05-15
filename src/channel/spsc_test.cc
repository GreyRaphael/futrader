#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "spsc.hpp"

#include <doctest/doctest.h>

#include <memory>
#include <print>
#include <thread>

TEST_CASE("testing the factorial function") {
    auto ptr = std::make_shared<lockfree::SPSC<std::optional<int>, 16>>();

    std::jthread producer{[ptr] {
        for (auto i = 0; i < 10; ++i) {
            while (!ptr->push(i)) {
                std::println("full, cannot push");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        ptr->push({});
    }};

    std::jthread consumer{[ptr] {
        while (true) {
            auto value = ptr->pop();
            if (!value) {
                std::println("empty, cannot pop");
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                continue;
            }

            if (*value == std::nullopt) break;
            std::println("consumer got {}", value.value().value());
        }
    }};
}