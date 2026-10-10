#pragma once

#include "commands.h"

#include <tlm>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#include <vector>

struct CommandPayload : public tlm::tlm_generic_payload {
    enum class Operation {
        Push,
        Pop,
    };

    enum class Response {
        Incomplete,
        Ok,
        Full,
        Empty,
        Invalid,
    };

    Operation operation = Operation::Push;
    std::vector<Command> commands;
    std::size_t request_count = 0;
    Response response = Response::Incomplete;
};

struct CommandProtocolTypes {
    using tlm_payload_type = CommandPayload;
    using tlm_phase_type = tlm::tlm_phase;
};
