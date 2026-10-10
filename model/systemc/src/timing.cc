#include "timing.h"

#include <stdexcept>
#include <string>

sc_core::sc_time clock_period_from_hz(uint64_t clock_freq_hz) {
    if (clock_freq_hz == 0) {
        throw std::invalid_argument("Clock frequency must be nonzero (received 0 Hz)");
    }

    const sc_core::sc_time period(1.0 / static_cast<double>(clock_freq_hz),
                                  sc_core::SC_SEC);
    if (period == sc_core::SC_ZERO_TIME) {
        throw std::invalid_argument(
            "Clock period for " + std::to_string(clock_freq_hz) +
            " Hz rounds to zero at the simulation time resolution");
    }
    return period;
}
