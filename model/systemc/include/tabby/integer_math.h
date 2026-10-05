#pragma once

#include <cstdint>
#include <stdexcept>

constexpr int64_t ceil_div(int64_t numerator, int64_t denominator) {
    if (denominator == 0) {
        throw std::invalid_argument("ceil_div denominator must be nonzero");
    }

    return numerator / denominator + (numerator % denominator != 0);
}
