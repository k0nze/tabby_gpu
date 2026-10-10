#pragma once

#include "command_queue.h"
#include "frame_buffer.h"
#include "input_buffer.h"

#include <string>

struct ModelConfig {
    InputBufferConfig input_buffer;
    FrameBufferConfig frame_buffer;
    CommandQueueConfig command_queue;
};

ModelConfig load_config(const std::string& path);
