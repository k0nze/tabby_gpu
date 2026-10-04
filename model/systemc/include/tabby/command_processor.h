#pragma once

#include "command_queue.h"
#include "input_buffer.h"

class CommandProcessor {
   public:
    CommandProcessor(CommandQueue& cmd_queue, InputBuffer& input_buffer);
    void process_next();

   private:
    CommandQueue& cmd_queue_;
    InputBuffer& input_buffer_;
};
