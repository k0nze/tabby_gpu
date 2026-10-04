#pragma once

#include "buffer_interface.h"
#include "vertex_2d.h"

void write_vertex_2d(BufferInterface& buffer, size_t address,
                     const Vertex2D& vertex_2d);
Vertex2D read_vertex_2d(BufferInterface& buffer, size_t address);
