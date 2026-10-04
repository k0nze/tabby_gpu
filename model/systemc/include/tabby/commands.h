#pragma once

#include <cstdint>
#include <variant>

struct CommandDrawTriangle {
    uint64_t vertices_2d_start_address;
};

struct CommandClearFrameBuffer {};

using Command = std::variant<CommandDrawTriangle, CommandClearFrameBuffer>;
