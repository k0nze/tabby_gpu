#include "vertex_2d_io.h"

#include "vertex_2d.h"

void write_vertex_2d(BufferInterface& buffer, size_t address,
                     const Vertex2D& vertex_2d) {
    // x
    buffer.write(address + 0, static_cast<uint8_t>(vertex_2d.x >> 0));
    buffer.write(address + 1, static_cast<uint8_t>(vertex_2d.x >> 8));
    buffer.write(address + 2, static_cast<uint8_t>(vertex_2d.x >> 16));
    buffer.write(address + 3, static_cast<uint8_t>(vertex_2d.x >> 24));

    // y
    buffer.write(address + 4, static_cast<uint8_t>(vertex_2d.y >> 0));
    buffer.write(address + 5, static_cast<uint8_t>(vertex_2d.y >> 8));
    buffer.write(address + 6, static_cast<uint8_t>(vertex_2d.y >> 16));
    buffer.write(address + 7, static_cast<uint8_t>(vertex_2d.y >> 24));

    // color
    buffer.write(address + 8, vertex_2d.color.red);
    buffer.write(address + 9, vertex_2d.color.green);
    buffer.write(address + 10, vertex_2d.color.blue);
}

Vertex2D read_vertex_2d(BufferInterface& buffer, size_t address) {
    Vertex2D vertex_2d{};

    // x
    vertex_2d.x = static_cast<uint8_t>((buffer.read(address + 0) << 0)) |
                  static_cast<uint8_t>((buffer.read(address + 1) << 8)) |
                  static_cast<uint8_t>((buffer.read(address + 2) << 16)) |
                  static_cast<uint8_t>((buffer.read(address + 3) << 24));

    // y
    vertex_2d.y = static_cast<uint8_t>((buffer.read(address + 4) << 0)) |
                  static_cast<uint8_t>((buffer.read(address + 5) << 8)) |
                  static_cast<uint8_t>((buffer.read(address + 6) << 16)) |
                  static_cast<uint8_t>((buffer.read(address + 7) << 24));

    // color
    vertex_2d.color.red = buffer.read(address + 8);
    vertex_2d.color.green = buffer.read(address + 9);
    vertex_2d.color.blue = buffer.read(address + 10);

    return vertex_2d;
}
