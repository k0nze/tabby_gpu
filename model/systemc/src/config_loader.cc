#include "config_loader.h"

#include <yaml-cpp/yaml.h>

#include <stdexcept>

namespace {
template <typename T>
T read_unsigned(const YAML::Node& node, const char* key) {
    const auto value = node[key];
    if (!value.IsScalar() || value.Scalar().empty() || value.Scalar().front() == '-') {
        throw std::invalid_argument(
            std::string("Missing or invalid nonnegative setting: ") + key);
    }
    return value.as<T>();
}
}  // namespace

InputBufferConfig load_input_buffer_config(const std::string& path) {
    const auto root = YAML::LoadFile(path);
    const auto buffer = root["input_buffer"];
    if (!buffer.IsMap()) {
        throw std::invalid_argument("Configuration requires an input_buffer mapping");
    }
    return {
        read_unsigned<size_t>(buffer, "capacity_bytes"),
        read_unsigned<uint64_t>(buffer, "clock_frequency_hz"),
        read_unsigned<uint64_t>(buffer, "read_setup_cycles"),
        read_unsigned<uint64_t>(buffer, "read_cycles_per_word"),
        read_unsigned<size_t>(buffer, "bytes_per_word"),
    };
}
