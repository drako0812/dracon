#pragma once

#include "types.hpp"

namespace dracon::console {

    // constexpr i64 FRAMEBUFFER_PIX_WIDTH  = 192;
    // constexpr i64 FRAMEBUFFER_PIX_HEIGHT = 192;
    constexpr i64 FRAMEBUFFER_PIX_WIDTH  = 320;
    constexpr i64 FRAMEBUFFER_PIX_HEIGHT = 320;
    constexpr i64 FRAMEBUFFER_BPP        = 5;
    constexpr i64 FRAMEBUFFER_PIXELS     = FRAMEBUFFER_PIX_WIDTH * FRAMEBUFFER_PIX_HEIGHT;
    constexpr i64 FRAMEBUFFER_BITS       = FRAMEBUFFER_PIXELS * FRAMEBUFFER_BPP;
    constexpr i64 FRAMEBUFFER_SIZE       = FRAMEBUFFER_BITS / 8;
    constexpr i64 PALETTE_SIZE           = 24;
    constexpr i64 FONT_GLYPH_COUNT       = 256;
    constexpr i64 SPRITE_PIX_WIDTH       = 8;
    constexpr i64 SPRITE_PIX_HEIGHT      = 8;
    constexpr i64 SPRITE_PIXELS          = SPRITE_PIX_WIDTH * SPRITE_PIX_HEIGHT;
    constexpr i64 SPRITE_BITS            = SPRITE_PIXELS * FRAMEBUFFER_BPP;
    constexpr i64 SPRITE_SIZE            = SPRITE_BITS / 8;
    constexpr i64 SPRITE_MEMORY_SIZE     = 2048;

} // namespace dracon::console
