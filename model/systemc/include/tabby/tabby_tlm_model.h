#pragma once

#include "command_processor.h"

#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_module_name.h>
#include <systemc>

class TabbyTLMModel : public sc_core::sc_module {
   public:
    SC_HAS_PROCESS(TabbyTLMModel);
    TabbyTLMModel(sc_core::sc_module_name name, CommandProcessor& cmd_proc);

   private:
    CommandProcessor& cmd_proc_;
    void run();
};
