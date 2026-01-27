#pragma once

#include <cstring>
#include <format>
#include <iostream>
#include "../types.hpp"

namespace dracon {
    class Console;
}

namespace dracon::console {

    struct PaletteEntry {
        u8 R;
        u8 G;
        u8 B;
    };

    constexpr inline auto PAL(u32 value) -> PaletteEntry {
        auto ret = PaletteEntry{
          .R = (u8)((value >> 16) & 0xFF),
          .G = (u8)((value >> 8) & 0xFF),
          .B = (u8)(value & 0xFF),
        };
        return ret;
    }

    struct PaletteData {
        PaletteEntry Palette[24];

        constexpr PaletteData() : Palette{} { memset((void *)Palette, 0, sizeof(Palette)); }
        static constexpr auto Default() -> PaletteData;
        auto                  SetEntry(Console * console, u8 index, u8 red, u8 green, u8 blue) -> void;
        auto                  GetEntry(u8 index) const -> PaletteEntry;
        auto                  LoadDefault(Console * console) -> void;
    };

} // namespace dracon::console
