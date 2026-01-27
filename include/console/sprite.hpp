#pragma once

#include <array>
#include <bitset>
#include <optional>
#include <string_view>
#include "./const.hpp"

namespace dracon::console {

    struct Sprite {
        std::bitset<SPRITE_PIXELS * FRAMEBUFFER_BPP> Pixels = {};

        auto                                         SetPixel(i32 x, i32 y, i32 color) -> void;
        auto                                         GetPixel(i32 x, i32 y) const -> std::optional<i32>;
    };

    struct PaletteData;

    struct SpriteData {
        std::array<Sprite, SPRITE_MEMORY_SIZE> Sprites = {};

        auto                                   LoadSpriteData(const PaletteData & pal, const std::string_view file) -> bool;
    };

} // namespace dracon::console
