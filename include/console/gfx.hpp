#pragma once

#include "./fb.hpp"
#include "./font.hpp"
#include "./pal.hpp"
#include "./sprite.hpp"

namespace dracon::console {

    struct Gfx {
        FrameBuffer FrameBuf = {};
        PaletteData PalData  = {};
        FontData    FontData = {};
        SpriteData  SprData  = {};
    };

} // namespace dracon::console
