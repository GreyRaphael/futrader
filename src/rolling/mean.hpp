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

struct Mean {
    /* data */
};

struct Sum {
    /* data */
};

struct Std {
    /* data */
};
}  // namespace rolling