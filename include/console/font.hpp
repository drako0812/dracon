#pragma once

#include <array>
#include <bitset>
#include "console/const.hpp"

namespace dracon::console {

    struct FontGlyph {
        std::bitset<SPRITE_PIXELS> Data = {};
    };

    struct FontData {
        std::array<FontGlyph, FONT_GLYPH_COUNT> Glyphs = {};

        auto                                    LoadDefault() -> void;
    };

} // namespace dracon::console
