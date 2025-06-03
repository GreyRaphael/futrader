#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "mpsc.hpp"

#include <doctest/doctest.h>

#include <print>
#include <thread>

constexpr size_t N = 10000000;
constexpr size_t WORK_NUM = 8;

TEST_CASE("optional") {
    lockfree::MPSC<int, 1024> channel{};
    std::vector<std::jthread> producers;
    producers.reserve(WORK_NUM);
    for (auto i = 0; i < WORK_NUM; ++i) {
        producers.emplace_back([&channel, i] {
            for (auto j = i * (N / WORK_NUM); j < (i + 1) * (N / WORK_NUM); ++j) {
                while (!channel.push(j)) continue;
            }
            while (!channel.push(-1)) continue;  // signal end
        });
    }

    std::jthread consumer{[&channel] {
        auto start = std::chrono::high_resolution_clock::now();
        size_t count{0};
        size_t sum{0};
        size_t finished_producers{0};
        while (finished_producers < WORK_NUM) {
            auto value = channel.pop();
            if (!value) continue;

            if (value.value() == -1) {
                ++finished_producers;
                continue;
            }
            // std::println("consumer got {}", value.value());
            ++count;
            sum += value.value();
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
        std::println("consumer processed {} items, sum={}, costs={}ms", count, sum, elapsed.count());
        REQUIRE_EQ(count, N);
        REQUIRE_EQ(sum, (N * (N - 1)) / 2);
    }};
}

TEST_CASE("reference") {
    lockfree::MPSC<int, 1024> channel{};
    std::vector<std::jthread> producers;
    producers.reserve(WORK_NUM);
    for (auto i = 0; i < WORK_NUM; ++i) {
        producers.emplace_back([&channel, i] {
            for (auto j = i * (N / WORK_NUM); j < (i + 1) * (N / WORK_NUM); ++j) {
                while (!channel.push(j)) continue;
            }
            while (!channel.push(-1)) continue;  // signal end
        });
    }

    std::jthread consumer{[&channel] {
        auto start = std::chrono::high_resolution_clock::now();
        size_t count{0};
        size_t sum{0};
        size_t finished_producers{0};
        int value{};
        while (finished_producers < WORK_NUM) {
            if (!channel.pop(value)) continue;

            if (value == -1) {
                ++finished_producers;
                continue;
            }
            // std::println("consumer got {}", value.value());
            ++count;
            sum += value;
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
        std::println("consumer processed {} items, sum={}, costs={}ms", count, sum, elapsed.count());
        REQUIRE_EQ(count, N);
        REQUIRE_EQ(sum, (N * (N - 1)) / 2);
    }};
}

TEST_CASE("inside") {
    lockfree::MPSC<int, 1024> channel{};
    std::vector<std::jthread> producers;
    producers.reserve(WORK_NUM);
    for (auto i = 0; i < WORK_NUM; ++i) {
        producers.emplace_back([&channel, i] {
            for (auto j = i * (N / WORK_NUM); j < (i + 1) * (N / WORK_NUM); ++j) {
                while (!channel.push(j)) continue;
            }
            while (!channel.push(-1)) continue;  // signal end
        });
    }

    std::jthread consumer{[&channel] {
        auto start = std::chrono::high_resolution_clock::now();
        size_t count{0};
        size_t sum{0};
        size_t finished_producers{0};
        while (finished_producers < WORK_NUM) {
            int value{};
            if (!channel.pop(value)) continue;

            if (value == -1) {
                ++finished_producers;
                continue;
            }
            // std::println("consumer got {}", value.value());
            ++count;
            sum += value;
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
        std::println("consumer processed {} items, sum={}, costs={}ms", count, sum, elapsed.count());
        REQUIRE_EQ(count, N);
        REQUIRE_EQ(sum, (N * (N - 1)) / 2);
    }};
}