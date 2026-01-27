#pragma once

#include <bitset>
#include <optional>
#include "./const.hpp"
#include "types.hpp"
#include "util.hpp"

namespace dracon {
    class Console;
}

namespace dracon::console {

    struct FrameBuffer {
        std::bitset<FRAMEBUFFER_PIXELS * FRAMEBUFFER_BPP> Buffer;

        auto                                              SetPixelFast(Console * console, i32 x, i32 y, i32 color) -> void;
        auto                                              SetPixel(Console * console, i32 x, i32 y, i32 color) -> void;
        auto                                              GetPixelFast(i32 x, i32 y) const -> i32;
        auto                                              GetPixel(i32 x, i32 y) const -> std::optional<i32>;
        auto                                              Cls(Console * console, i32 color) -> void;
        auto                                              HLine(Console * console, i32 x, i32 y, i32 l, i32 color) -> void;
        auto                                              VLine(Console * console, i32 x, i32 y, i32 l, i32 color) -> void;
        auto Line(Console * console, i32 x1, i32 y1, i32 x2, i32 y2, i32 color) -> void;
        auto Rect(Console * console, i32 x, i32 y, i32 w, i32 h, i32 color) -> void;
        auto RectF(Console * console, i32 x, i32 y, i32 w, i32 h, i32 color) -> void;
        auto Circ(Console * console, i32 x, i32 y, i32 r, i32 color) -> void;
        auto CircF(Console * console, i32 x, i32 y, i32 r, i32 color) -> void;
        auto PutCh(Console * console, i32 x, i32 y, u8 ch, i32 color) -> void;
        auto PutS(Console * console, i32 x, i32 y, const std::string_view str, i32 color) -> void;
        auto Tri(Console * console, i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, i32 color) -> void;
        auto TriF(Console * console, i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, i32 color) -> void;
        // auto Spr(Console * console, i32 x, i32 y, i32 spr, std::optional<i32> key = std::nullopt) -> void;
        auto Spr(Console * console, i32 x, i32 y, i32 spr, i32 flip, i32 rot, std::optional<i32> key = std::nullopt) -> void;
    };

} // namespace dracon::console
