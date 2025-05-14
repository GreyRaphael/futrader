#pragma once

#include <cmath>
#include <cstddef>
#include <vector>

namespace rolling {

inline bool nan_or_inf(std::floating_point auto x) { return x - x != 0; }

struct Container {
    Container(size_t n) : buf(n, NAN) {}

    std::vector<double> buf;
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

struct Sumer {
    Sumer(size_t n) : container(n), nan_count(n) {}
    Container container;
    size_t nan_count;
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

struct Meaner {
    Meaner(size_t n) : sumer(n) {}
    Sumer sumer;

    double update(double new_val) {
        return sumer.update(new_val) / sumer.container.len();
    }
};

struct Stder {
    Stder(size_t n) : sumer(n), sq_sumer(n), num(n) {}
    Sumer sumer;
    Sumer sq_sumer;
    size_t num;

    double update(double new_val) {
        auto sum = sumer.update(new_val);
        auto sq_sum = sq_sumer.update(new_val * new_val);
        auto variance = (sq_sum - sum * sum / num) / (num - 1);
        return sqrt(variance);
    }
};

struct Skewer {
    Skewer(size_t n) : meaner(n), sq_sumer(n), cb_sumer(n), num(n) {}

    Meaner meaner;
    Sumer sq_sumer;
    Sumer cb_sumer;
    size_t num;

    double update(double new_val) {
        auto mean = meaner.update(new_val);
        auto sq_sum = sq_sumer.update(new_val * new_val);
        auto cb_sum = cb_sumer.update(new_val * new_val * new_val);
        auto variance = sq_sum / num - mean * mean;

        return (cb_sum / num - 3.0 * mean * variance - mean * mean * mean) / std::pow(variance, 1.5);
    }
};

}  // namespace rolling