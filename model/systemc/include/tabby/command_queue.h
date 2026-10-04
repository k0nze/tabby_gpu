#pragma once

#include <queue>

#include "commands.h"

class CommandQueue {
   public:
    void push(const Command& command);
    Command pop();
    bool empty() const;

   private:
    std::queue<Command> queue_;
};
