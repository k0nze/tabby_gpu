#pragma once

#include "command_processor.h"
#include "command_queue.h"
#include "frame_buffer.h"
#include "input_buffer.h"
#include "triangle_rasterizer.h"

#include <string>

struct ModelConfig {
    InputBufferConfig input_buffer;
    FrameBufferConfig frame_buffer;
    CommandQueueConfig command_queue;
    CommandProcessorConfig command_processor;
    TriangleRasterizerConfig triangle_rasterizer_config;
};

ModelConfig load_config(const std::string& path);
