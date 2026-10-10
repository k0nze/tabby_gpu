#pragma once

#include <tlm>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>

struct FrameBufferPayload : public tlm::tlm_generic_payload {
    enum class Operation {
        Write,
        Clear,
    };

    Operation operation = Operation::Write;
};

struct FrameBufferProtocolTypes {
    using tlm_payload_type = FrameBufferPayload;
    using tlm_phase_type = tlm::tlm_phase;
};
