#include "config_loader.h"

#include <stdexcept>
#include <yaml-cpp/yaml.h>

namespace {
template <typename T>
T read_unsigned(const YAML::Node& node, const char* key) {
    const auto value = node[key];
    if (!value.IsScalar() || value.Scalar().empty() || value.Scalar().front() == '-') {
        throw std::invalid_argument(
            std::string("Missing or invalid non-negative setting: ") + key);
    }
    return value.as<T>();
}
InputBufferConfig load_input_buffer_config(const YAML::Node& root) {
    const auto buffer = root["input_buffer"];
    if (!buffer.IsMap()) {
        throw std::invalid_argument("Configuration requires an input_buffer mapping");
    }
    return {
        read_unsigned<size_t>(buffer, "capacity_bytes"),
        read_unsigned<uint64_t>(buffer, "clock_freq_hz"),
        read_unsigned<uint64_t>(buffer, "read_setup_cycles"),
        read_unsigned<uint64_t>(buffer, "read_cycles_per_word"),
        read_unsigned<size_t>(buffer, "bytes_per_word"),
    };
}

FrameBufferConfig load_frame_buffer_config(const YAML::Node& root) {
    const auto buffer = root["frame_buffer"];
    if (!buffer.IsMap()) {
        throw std::invalid_argument("Configuration requires a frame_buffer mapping");
    }
    return {
        read_unsigned<size_t>(buffer, "width"),
        read_unsigned<size_t>(buffer, "height"),
        read_unsigned<uint64_t>(buffer, "clock_freq_hz"),
        read_unsigned<uint64_t>(buffer, "write_setup_cycles"),
        read_unsigned<uint64_t>(buffer, "write_cycles_per_word"),
        read_unsigned<size_t>(buffer, "bytes_per_word"),
    };
}
}  // namespace

ModelConfig load_config(const std::string& path) {
    const auto root = YAML::LoadFile(path);
    if (!root.IsMap()) {
        throw std::invalid_argument("Configuration root must be a mapping");
    }
    return {load_input_buffer_config(root), load_frame_buffer_config(root)};
}
