#include <chrono>
#include <cstddef>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <memory>
#include <print>
#include <thread>

#include "spsc.hpp"

TEST_CASE("simple") {
    auto ptr = std::make_shared<lockfree::SPSC<std::optional<int>, 16>>();

    std::jthread producer{[ptr] {
        for (auto i = 0; i < 10; ++i) {
            while (!ptr->push(i)) {
                std::println("full, cannot push");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        ptr->push(std::nullopt);
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
            // value->value() == value.value().value()
            std::println("consumer got {}", value->value());
        }
    }};
}

constexpr size_t N = 100000000;

// stack channel it faster than heap channel
TEST_CASE("stack") {
    lockfree::SPSC<int, 1024> channel{};

    std::jthread producer{[&channel] {
        for (auto i = 0; i < N; ++i) {
            while (!channel.push(i)) continue;
        }
        while (!channel.push(-1)) continue;
    }};

    std::jthread consumer{[&channel] {
        auto start = std::chrono::high_resolution_clock::now();
        size_t count{0};
        size_t sum{0};
        while (true) {
            auto value = channel.pop();
            if (!value) continue;

            if (value.value() == -1) break;
            // std::println("consumer got {}", value.value());
            sum += value.value();
            ++count;
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
        std::println("consumer processed {} items, sum={}, costs={}ms", count, sum, elapsed.count());
        REQUIRE_EQ(count, N);
        REQUIRE_EQ(sum, N * (N - 1) / 2);
    }};
}

TEST_CASE("heap") {
    auto ptr = std::make_unique<lockfree::SPSC<int, 1024>>();

    std::jthread producer{[&ptr] {
        for (auto i = 0; i < N; ++i) {
            while (!ptr->push(i)) continue;
        }
        while (!ptr->push(-1)) continue;
    }};

    std::jthread consumer{[&ptr] {
        auto start = std::chrono::high_resolution_clock::now();
        size_t count{0};
        size_t sum{0};
        while (true) {
            auto value = ptr->pop();
            if (!value) continue;

            if (value.value() == -1) break;
            // std::println("consumer got {}", value.value());
            sum += value.value();
            ++count;
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
        std::println("consumer processed {} items, sum={}, costs={}ms", count, sum, elapsed.count());
        REQUIRE_EQ(count, N);
        REQUIRE_EQ(sum, N * (N - 1) / 2);
    }};
}