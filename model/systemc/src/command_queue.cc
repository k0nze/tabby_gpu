#include "command_queue.h"

#include "commands.h"

#include <stdexcept>

void CommandQueue::push(const Command& command) { queue_.push(command); }

Command CommandQueue::pop() {
    if (queue_.empty()) {
        throw std::underflow_error("CommandQueue");
    }
    auto command = queue_.front();
    queue_.pop();
    return command;
}

bool CommandQueue::empty() const { return queue_.empty(); }
