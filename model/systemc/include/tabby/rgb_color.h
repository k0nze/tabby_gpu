#pragma once

#include <cstddef>
#include <cstdint>

inline constexpr std::size_t RGB_COLOR_SIZE = 3;

struct RGBColor {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

inline constexpr RGBColor RGB_BLACK{0, 0, 0};
inline constexpr RGBColor RGB_WHITE{255, 255, 255};
inline constexpr RGBColor RGB_RED{255, 0, 0};
inline constexpr RGBColor RGB_GREEN{0, 255, 0};
inline constexpr RGBColor RGB_BLUE{0, 0, 255};
inline constexpr RGBColor RGB_YELLOW{255, 255, 0};
inline constexpr RGBColor RGB_CYAN{0, 255, 255};
inline constexpr RGBColor RGB_MAGENTA{255, 0, 255};
inline constexpr RGBColor RGB_GRAY{128, 128, 128};
