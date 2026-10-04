// #include <systemc>
#include "command_processor.h"
#include "command_queue.h"
#include "commands.h"
#include "input_buffer.h"
#include "vertex_2d.h"
#include "vertex_2d_io.h"

int main(int argc, char** argv) {
    InputBuffer input_buffer(1024);
    CommandQueue cmd_queue;
    CommandProcessor cmd_proc(cmd_queue, input_buffer);

    Vertex2D v0{10, 10, {255, 0, 0}};
    Vertex2D v1{50, 10, {0, 255, 0}};
    Vertex2D v2{30, 40, {0, 0, 255}};

    write_vertex_2d(input_buffer, 0x100, v0);
    write_vertex_2d(input_buffer, 0x10B, v0);
    write_vertex_2d(input_buffer, 0x116, v0);

    cmd_queue.push(CommandDrawTriangle{0x100});

    cmd_proc.process_next();

    return 0;
}

/*
int sc_main(int argc, char* argv[]) {
    sc_core::sc_start();
    return 0;
}
*/
