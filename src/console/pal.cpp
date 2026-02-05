#include <cstring>
#include "console.hpp"
#include "console/pal.hpp"
#include "util.hpp"

namespace dracon::console {

    constexpr auto PaletteData::Default() -> PaletteData {
        PaletteData        ret{};
        const PaletteEntry pal[] = {
          PAL(0x000000), PAL(0x000080), PAL(0x008000), PAL(0x008080), PAL(0x800000), PAL(0x800080),
          PAL(0x808000), PAL(0x909090), PAL(0x303030), PAL(0x0000FF), PAL(0x00FF00), PAL(0x00FFFF),
          PAL(0xFF0000), PAL(0xFF00FF), PAL(0xFFFF00), PAL(0xC0C0C0), PAL(0x606060), PAL(0x6000C0),
          PAL(0x00C060), PAL(0x0060C0), PAL(0xC06000), PAL(0xC00060), PAL(0x60C000), PAL(0xFFFFFF),
        };
        memcpy(ret.Palette, pal, sizeof(ret.Palette));
        return ret;
    }
    auto PaletteData::SetEntry(Console * console, u8 index, u8 red, u8 green, u8 blue) -> void {
        Palette[index] = PaletteEntry{.R = red, .G = green, .B = blue};
        console->PaletteDirty();
    }
    auto PaletteData::GetEntry(u8 index) const -> PaletteEntry { return Palette[index]; }
    auto PaletteData::LoadDefault(Console * console) -> void {
        auto p = Default();
        memcpy(Palette, p.Palette, sizeof(Palette));
        console->PaletteDirty();
    }

} // namespace dracon::console
