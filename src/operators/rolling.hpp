#pragma once

#include <array>
#include <cmath>
#include <cstddef>

namespace rolling {

inline bool nan_or_inf(std::floating_point auto x) { return x - x != 0; }

// make NAN array in compile-time
template <size_t N>
constexpr std::array<double, N> make_nan_array() {
    std::array<double, N> arr{};
    arr.fill(NAN);
    return arr;
}

template <size_t N>
struct Container {
    std::array<double, N> buf = make_nan_array<N>();
    size_t head_idx{0};
    size_t tail_idx{0};

    auto len() { return buf.size(); }
    auto head() { return buf[head_idx]; }
    auto tail() { return buf[tail_idx]; }
    auto get(size_t idx) { return buf[(head_idx + idx) % len()]; }
    void update(double new_val) {
        tail_idx = head_idx;
        buf[tail_idx] = new_val;
        head_idx = (head_idx + 1) % len();
    }
};

template <size_t N>
struct Sumer {
    Container<N> container{};
    size_t nan_count{N};
    double sum{0};

    double update(double new_val) {
        auto old_val = container.head();
        container.update(new_val);

        if (nan_or_inf(old_val)) {
            nan_count -= 1;
        } else {
            sum -= old_val;
        }

        if (nan_or_inf(new_val)) {
            nan_count += 1;
        } else {
            sum += new_val;
        }

        return nan_count > 0 ? NAN : sum;
    }
};

template <size_t N>
struct Meaner {
    Sumer<N> sumer{};

    double update(double new_val) {
        return sumer.update(new_val) / sumer.container.len();
    }
};

template <size_t N>
struct Stder {
    Sumer<N> sumer{};
    Sumer<N> sq_sumer{};

    double update(double new_val) {
        auto sum = sumer.update(new_val);
        auto sq_sum = sq_sumer.update(new_val * new_val);
        auto variance = (sq_sum - sum * sum / N) / (N - 1);
        return sqrt(variance);
    }
};

template <size_t N>
struct Skewer {
    Meaner<N> meaner{};
    Sumer<N> sq_sumer{};
    Sumer<N> cb_sumer{};

    double update(double new_val) {
        auto mean = meaner.update(new_val);
        auto sq_sum = sq_sumer.update(new_val * new_val);
        auto cb_sum = cb_sumer.update(new_val * new_val * new_val);
        auto variance = sq_sum / N - mean * mean;

        return (cb_sum / N - 3.0 * mean * variance - mean * mean * mean) / std::pow(variance, 1.5);
    }
};

}  // namespace rolling