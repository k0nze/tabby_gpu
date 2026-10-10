#include "command_payload.h"
#include "command_queue.h"
#include "commands.h"
#include "timing.h"

#include <algorithm>
#include <stdexcept>
#include <sysc/kernel/sc_module_name.h>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#include <vector>

CommandQueue::CommandQueue(sc_core::sc_module_name name,
                           const CommandQueueConfig& config)
    : sc_core::sc_module(name), socket("socket"), config_(config) {
    // check that config values are valid (greater than 0)
    if (config_.size < 1) {
        throw std::invalid_argument("CommandQueue size must be greater than zero");
    }

    // compute clock period
    clock_period_ = clock_period_from_hz(config_.clock_freq_hz);

    // compute latencies
    write_setup_latency_ =
        clock_period_ * static_cast<double>(config_.write_setup_cycles);
    write_command_latency_ =
        clock_period_ * static_cast<double>(config_.write_cycles_per_command);
    read_setup_latency_ =
        clock_period_ * static_cast<double>(config_.read_setup_cycles);
    read_command_latency_ =
        clock_period_ * static_cast<double>(config_.read_cycles_per_command);

    socket.register_b_transport(this, &CommandQueue::b_transport);
}

void CommandQueue::push(const Command& command) {
    if (queue_.size() >= config_.size) {
        throw std::overflow_error("CommandQueue is full");
    }
    queue_.push(command);
}

Command CommandQueue::pop() {
    if (queue_.empty()) {
        throw std::underflow_error("CommandQueue");
    }
    auto command = queue_.front();
    queue_.pop();
    return command;
}

bool CommandQueue::empty() const { return queue_.empty(); }

void CommandQueue::b_transport(CommandPayload& trans, sc_core::sc_time& delay) {
    trans.response = CommandPayload::Response::Incomplete;

    if (trans.operation == CommandPayload::Operation::Push) {
        // check if the push command is empty
        if (trans.commands.empty()) {
            trans.response = CommandPayload::Response::Invalid;
            return;
        }

        // check if commands pushed fit into queue
        if (trans.commands.size() > config_.size - queue_.size()) {
            trans.response = CommandPayload::Response::Full;
            return;
        }

        // add commands to queue
        for (const auto& command : trans.commands) {
            queue_.push(command);
        }

        delay += write_setup_latency_ +
                 write_command_latency_ * static_cast<double>(trans.commands.size());

        trans.response = CommandPayload::Response::Ok;
        return;
    }

    else if (trans.operation == CommandPayload::Operation::Pop) {
        // check if 0 commands where requested
        if (trans.request_count == 0) {
            trans.response = CommandPayload::Response::Invalid;
            return;
        }

        // check if there are command available
        if (queue_.empty()) {
            trans.commands.clear();
            trans.response = CommandPayload::Response::Empty;
            return;
        }

        const auto count = std::min(trans.request_count, queue_.size());

        std::vector<Command> commands;
        commands.reserve(count);

        for (size_t i = 0; i < count; i++) {
            commands.push_back(queue_.front());
            queue_.pop();
        }

        trans.commands = std::move(commands);

        delay +=
            read_setup_latency_ + read_command_latency_ * static_cast<double>(count);

        trans.response = CommandPayload::Response::Ok;
        return;
    }

    trans.response = CommandPayload::Response::Invalid;
}
