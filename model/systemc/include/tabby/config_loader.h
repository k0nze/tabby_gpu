#pragma once

#include "frame_buffer.h"
#include "input_buffer.h"

#include <string>

struct ModelConfig {
    InputBufferConfig input_buffer;
    FrameBufferConfig frame_buffer;
};

ModelConfig load_config(const std::string& path);
