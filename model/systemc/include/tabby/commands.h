#pragma once

#include <cstdint>
#include <variant>

struct CommandClearFrameBuffer {};

struct CommandDrawTriangle {
    uint64_t vertices_2d_start_address;
};

using Command = std::variant<CommandDrawTriangle, CommandClearFrameBuffer>;
