#include "console/font.hpp"
#include <array>
#include <bitset>
#include <format>
#include <iostream>

#ifndef PRINT
#    define PRINT(msg) std::cerr << (msg);
#endif

namespace dracon::console {

    static inline const std::array<u8, FONT_GLYPH_COUNT * SPRITE_PIX_HEIGHT> DEFAULT_FONT_DATA = {
#include "./font.dat"
    };

    auto BuildGlyph(const std::array<u8, FONT_GLYPH_COUNT * SPRITE_PIX_HEIGHT> & arr, std::size_t start_idx)
      -> std::bitset<SPRITE_PIXELS> {
        // PRINT(std::format("BuildGlyph {}: ", start_idx));
        std::bitset<SPRITE_PIXELS> ret{};
        for (std::size_t i = 0; i < 8; i++) {
            // PRINT(std::format("{}, ", i));
            auto b           = arr.at(start_idx + i);
            ret[(i * 8)]     = (b >> 7) & 1;
            ret[(i * 8) + 1] = (b >> 6) & 1;
            ret[(i * 8) + 2] = (b >> 5) & 1;
            ret[(i * 8) + 3] = (b >> 4) & 1;
            ret[(i * 8) + 4] = (b >> 3) & 1;
            ret[(i * 8) + 5] = (b >> 2) & 1;
            ret[(i * 8) + 6] = (b >> 1) & 1;
            ret[(i * 8) + 7] = (b >> 0) & 1;
        }
        // PRINT("\n");
        return ret;
    }

    auto FontData::LoadDefault() -> void {
        // PRINT("FontData::LoadDefault:\n");
        for (std::size_t i = 0; i < Glyphs.size(); i++) {
            // PRINT(std::format(" {}: ", i));
            auto gdat         = BuildGlyph(DEFAULT_FONT_DATA, i * (SPRITE_PIXELS / 8));
            Glyphs.at(i).Data = gdat;
        }
        // PRINT("\n");
    }

} // namespace dracon::console
