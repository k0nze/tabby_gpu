#include "command_processor.h"

#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_module_name.h>
#include <sysc/kernel/sc_time.h>
#include <sysc/utils/sc_report.h>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>

#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <vector>

#include "vertex_2d_io.h"

CommandProcessor::CommandProcessor(sc_core::sc_module_name name)
    : sc_core::sc_module(name), socket("socket") {}

std::vector<uint8_t> CommandProcessor::read_bytes(uint64_t address, size_t length) {
    // since tlm length is an unsigned int (32bit) and the length parameter is size_t
    // (64bit) a length check is needed
    if (length > std::numeric_limits<unsigned int>::max()) {
        throw std::length_error("TLM transfer length exceeds unsigned int");
    }

    tlm::tlm_generic_payload trans;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;

    std::vector<uint8_t> data(length);

    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_address(address);
    trans.set_data_ptr(data.data());
    trans.set_data_length(length);
    trans.set_streaming_width(length);
    trans.set_byte_enable_ptr(nullptr);
    trans.set_dmi_allowed(false);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    socket->b_transport(trans, delay);

    if (trans.is_response_error()) {
        SC_REPORT_ERROR("CommandProcessor", "TLM read failed");
    }

    sc_core::wait(delay);

    return data;
}
