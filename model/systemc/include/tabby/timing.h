#pragma once

#include <cstdint>
#include <systemc>

sc_core::sc_time clock_period_from_hz(uint64_t clock_freq_hz);
