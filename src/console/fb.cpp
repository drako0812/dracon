#include "console/fb.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <numeric>
#include "console.hpp"
#include "console/const.hpp"
#include "console/font.hpp"

namespace dracon::console {

#ifdef _WIN32
    inline static auto add_sat(u8 a, u8 b) -> u8 {
        u32 aa = static_cast<u32>(a);
        u32 bb = static_cast<u32>(b);
        u32 tmp = aa + bb;
        return static_cast<u8>(std::min(tmp, 0xFFU));
    }
    inline static auto sub_sat(u8 a, u8 b) -> u8 {
        i32 aa = static_cast<i32>(a);
        i32 bb = static_cast<i32>(b);
        i32 tmp = aa - bb;
        return static_cast<u8>(std::max(tmp, 0));
    }
#else
#define add_sat(a, b) std::add_sat((a), (b))
#define sub_sat(a, b) std::sub_sat((a), (b))
#endif

    enum PixBlendMode : u16 {
        BlendMode_NONE = 0b000000000,
        BlendMode_XOR  = 0b000100000,
        BlendMode_AND  = 0b001000000,
        BlendMode_OR   = 0b001100000,
        BlendMode_ADDW = 0b010000000,
        BlendMode_ADDS = 0b010100000,
        BlendMode_SUBW = 0b011000000,
        BlendMode_SUBS = 0b011100000,
        BlendMode_ROTL = 0b100000000,
        BlendMode_ROTR = 0b100100000,
        BlendMode_SHL  = 0b101000000,
        BlendMode_SHR  = 0b101100000,

        BlendMode_MASK  = 0b111100000,
        BlendMode_CMASK = 0b000011111,
    };

    auto FrameBuffer::SetPixelFast(Console * console, i32 x, i32 y, i32 color) -> void {
        std::size_t pidx = (y * FRAMEBUFFER_PIX_WIDTH) + x;
        std::size_t bidx = pidx * FRAMEBUFFER_BPP;

        auto        mode = static_cast<PixBlendMode>(color & BlendMode_MASK);

        i32         src_color = 0;

        if (mode != BlendMode_NONE) { src_color = GetPixelFast(x, y); }

        switch (mode) {
            case BlendMode_NONE:
                { /* Do Nothing */
                }
                break;
            case BlendMode_XOR:
                {
                    color = (color & BlendMode_CMASK) ^ src_color;
                }
                break;
            case BlendMode_AND:
                {
                    color = (color & BlendMode_CMASK) & src_color;
                }
                break;
            case BlendMode_OR:
                {
                    color = (color & BlendMode_CMASK) | src_color;
                }
                break;
            case BlendMode_ADDW:
                {
                    u8 colorb     = static_cast<u8>(color & BlendMode_CMASK);
                    u8 src_colorb = static_cast<u8>(src_color);
                    src_colorb += colorb;
                    color = src_colorb;
                }
                break;
            case BlendMode_ADDS:
                {
                    u8 colorb     = static_cast<u8>(color & BlendMode_CMASK);
                    u8 src_colorb = static_cast<u8>(src_color);
                    color         = add_sat(src_colorb, colorb);
                }
                break;
            case BlendMode_SUBW:
                {
                    u8 colorb     = static_cast<u8>(color & BlendMode_CMASK);
                    u8 src_colorb = static_cast<u8>(src_color);
                    src_colorb -= colorb;
                    color = src_colorb;
                }
                break;
            case BlendMode_SUBS:
                {
                    u8 colorb     = static_cast<u8>(color & BlendMode_CMASK);
                    u8 src_colorb = static_cast<u8>(src_color);
                    color         = sub_sat(src_colorb, colorb);
                }
                break;
            case BlendMode_ROTL:
                {
                    u8 colorb     = static_cast<u8>(color & BlendMode_CMASK);
                    u8 src_colorb = static_cast<u8>(src_color);
                    color         = std::rotl(src_colorb, colorb);
                }
                break;
            case BlendMode_ROTR:
                {
                    u8 colorb     = static_cast<u8>(color & BlendMode_CMASK);
                    u8 src_colorb = static_cast<u8>(src_color);
                    color         = std::rotr(src_colorb, colorb);
                }
                break;
            case BlendMode_SHL:
                {
                    u8 colorb     = static_cast<u8>(color & BlendMode_CMASK);
                    u8 src_colorb = static_cast<u8>(src_color);
                    color         = src_colorb << colorb;
                }
                break;
            case BlendMode_SHR:
                {
                    u8 colorb     = static_cast<u8>(color & BlendMode_CMASK);
                    u8 src_colorb = static_cast<u8>(src_color);
                    color         = src_colorb >> colorb;
                }
                break;
            default: UNREACHABLE; break;
        }

        Buffer[bidx]     = ((color >> 4) & 1) != 0;
        Buffer[bidx + 1] = ((color >> 3) & 1) != 0;
        Buffer[bidx + 2] = ((color >> 2) & 1) != 0;
        Buffer[bidx + 3] = ((color >> 1) & 1) != 0;
        Buffer[bidx + 4] = ((color >> 0) & 1) != 0;
        console->FrameBufferDirty();
    }

    auto FrameBuffer::SetPixel(Console * console, i32 x, i32 y, i32 color) -> void {
        if ((x < 0) || (x >= FRAMEBUFFER_PIX_WIDTH) || (y < 0) || (y >= FRAMEBUFFER_PIX_HEIGHT)) { return; }

        SetPixelFast(console, x, y, color);
    }

    auto FrameBuffer::GetPixelFast(i32 x, i32 y) const -> i32 {
        std::size_t pidx = (y * FRAMEBUFFER_PIX_WIDTH) + x;
        std::size_t bidx = pidx * FRAMEBUFFER_BPP;
        i32         ret  = 0;
        ret              = Buffer[bidx] ? 1 : 0;
        ret              = (ret << 1) | (Buffer[bidx + 1] ? 1 : 0);
        ret              = (ret << 1) | (Buffer[bidx + 2] ? 1 : 0);
        ret              = (ret << 1) | (Buffer[bidx + 3] ? 1 : 0);
        ret              = (ret << 1) | (Buffer[bidx + 4] ? 1 : 0);
        return ret;
    }
    auto FrameBuffer::GetPixel(i32 x, i32 y) const -> std::optional<i32> {
        if ((x < 0) || (x >= FRAMEBUFFER_PIX_WIDTH) || (y < 0) || (y >= FRAMEBUFFER_PIX_HEIGHT)) { return std::nullopt; }

        return GetPixelFast(x, y);
    }
    auto FrameBuffer::Cls(Console * console, i32 color) -> void {
        for (i32 y = 0; y < FRAMEBUFFER_PIX_HEIGHT; y++) {
            for (i32 x = 0; x < FRAMEBUFFER_PIX_WIDTH; x++) { SetPixelFast(console, x, y, color); }
        }
    }
    auto FrameBuffer::HLine(Console * console, i32 x, i32 y, i32 l, i32 color) -> void {
        i32 xx = x;
        while (xx < x + l) {
            SetPixel(console, xx, y, color);
            xx++;
        }
    }
    auto FrameBuffer::VLine(Console * console, i32 x, i32 y, i32 l, i32 color) -> void {
        i32 yy = y;
        while (yy < y + l) {
            SetPixel(console, x, yy, color);
            yy++;
        }
    }
    auto FrameBuffer::Line(Console * console, i32 x1, i32 y1, i32 x2, i32 y2, i32 color) -> void {
        i32       x1c  = x1;
        i32       y1c  = y1;
        const i32 dx   = std::abs(x2 - x1c);
        const i32 sx   = x1c < x2 ? 1 : -1;
        const i32 dy   = -std::abs(y2 - y1c);
        const i32 sy   = y1c < y2 ? 1 : -1;
        i32       e    = dx + dy;
        const i32 minx = std::min(x1, x2);
        const i32 miny = std::min(y1, y2);

        if ((dx == 0) && (dy == 0)) {
            SetPixel(console, x1c, y1c, color);
            return;
        }

        if ((dx == 0) && (dy != 0)) {
            const i32 yy = std::max(y1, y2) - std::min(y1, y2);
            VLine(console, minx, miny, yy, color);
            return;
        }

        if ((dy == 0) && (dx != 0)) {
            const i32 xx = std::max(x1, x2) - std::min(x1, x2);
            HLine(console, minx, miny, xx, color);
            return;
        }

        while (true) {
            SetPixel(console, x1c, y1c, color);
            const i32 e2 = 2 * e;
            if (e2 >= dy) {
                if (x1c == x2) { break; }
                e += dy;
                x1c += sx;
            }
            if (e2 <= dx) {
                if (y1c == y2) { break; }
                e += dx;
                y1c += sy;
            }
        }
    }
    auto FrameBuffer::Rect(Console * console, i32 x, i32 y, i32 w, i32 h, i32 color) -> void {
        Line(console, x, y, x + w, y, color);
        Line(console, x, y + h, x + w, y + h, color);
        Line(console, x, y, x, y + h, color);
        Line(console, x + w, y, x + w, y + h, color);
    }
    auto FrameBuffer::RectF(Console * console, i32 x, i32 y, i32 w, i32 h, i32 color) -> void {
        i32 xx = x;
        while (xx < x + w) {
            Line(console, xx, y, xx, y + h, color);
            xx++;
        }
    }
    auto FrameBuffer::Circ(Console * console, i32 x, i32 y, i32 r, i32 color) -> void {
        i32 f     = 1 - r;
        i32 ddF_x = 0;
        i32 ddF_y = -2 * r;
        i32 xx    = 0;
        i32 yy    = r;

        SetPixel(console, x, y + r, color);
        SetPixel(console, x, y - r, color);
        SetPixel(console, x + r, y, color);
        SetPixel(console, x - r, y, color);

        while (xx < yy) {
            if (f >= 0) {
                yy--;
                ddF_y += 2;
                f += ddF_y;
            }
            xx++;
            ddF_x += 2;
            f += ddF_x + 2;
            SetPixel(console, x + xx, y + yy, color);
            SetPixel(console, x - xx, y + yy, color);
            SetPixel(console, x + xx, y - yy, color);
            SetPixel(console, x - xx, y - yy, color);
            SetPixel(console, x + yy, y + xx, color);
            SetPixel(console, x - yy, y + xx, color);
            SetPixel(console, x + yy, y - xx, color);
            SetPixel(console, x - yy, y - xx, color);
        }
    }
    auto FrameBuffer::CircF(Console * console, i32 x, i32 y, i32 r, i32 color) -> void {
        i32 f     = 1 - r;
        i32 ddF_x = 0;
        i32 ddF_y = -2 * r;
        i32 xx    = 0;
        i32 yy    = r;

        SetPixel(console, x, y + r, color);
        SetPixel(console, x, y - r, color);
        SetPixel(console, x + r, y, color);
        SetPixel(console, x - r, y, color);
        Line(console, x - r, y, x + r, y, color);

        while (xx < yy) {
            if (f >= 0) {
                yy--;
                ddF_y += 2;
                f += ddF_y;
            }
            xx++;
            ddF_x += 2;
            f += ddF_x + 2;
            Line(console, x - xx, y + yy, x + xx, y + yy, color);
            Line(console, x - xx, y - yy, x + xx, y - yy, color);
            Line(console, x - yy, y + xx, x + yy, y + xx, color);
            Line(console, x - yy, y - xx, x + yy, y - xx, color);
        }
    }
    auto FrameBuffer::PutCh(Console * console, i32 x, i32 y, u8 ch, i32 color) -> void {
        const FontGlyph glyph = console->GetGfx().FontData.Glyphs.at(ch);

        for (i32 yy = 0; yy < SPRITE_PIX_HEIGHT; yy++) {
            for (i32 xx = 0; xx < SPRITE_PIX_WIDTH; xx++) {
                const std::size_t idx = (yy * SPRITE_PIX_WIDTH) + xx;
                if (glyph.Data[idx]) { SetPixel(console, x + xx, y + yy, color); }
            }
        }
    }
    auto FrameBuffer::PutS(Console * console, i32 x, i32 y, const std::string_view str, i32 color) -> void {
        for (std::size_t idx = 0; idx < str.length(); idx++) {
            PutCh(console, static_cast<i32>(x + (idx * SPRITE_PIX_WIDTH)), y, str.at(idx), color);
        }
    }

    auto FrameBuffer::Tri(Console * console, i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, i32 color) -> void {
        Line(console, x1, y1, x2, y2, color);
        Line(console, x2, y2, x3, y3, color);
        Line(console, x3, y3, x1, y1, color);
    }

    auto signedTriangleArea(i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3) -> double {
        return 0.5 * ((y2 - y1) * (x2 + x1) + (y3 - y2) * (x3 + x2) + (y1 - y3) * (x1 + x3));
    }

    auto FrameBuffer::TriF(Console * console, i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, i32 color) -> void {
        auto   bbminx     = std::min({x1, x2, x3});
        auto   bbminy     = std::min({y1, y2, y3});
        auto   bbmaxx     = std::max({x1, x2, x3});
        auto   bbmaxy     = std::max({y1, y2, y3});
        double total_area = signedTriangleArea(x1, y1, x2, y2, x3, y3);

        for (i32 y = bbminy; y < bbmaxy; y++) {
            for (i32 x = bbminx; x < bbmaxx; x++) {
                double alpha = signedTriangleArea(x, y, x2, y2, x3, y3) / total_area;
                double beta  = signedTriangleArea(x, y, x3, y3, x1, y1) / total_area;
                double gamma = signedTriangleArea(x, y, x1, y1, x2, y2) / total_area;
                if (alpha < 0 || beta < 0 || gamma < 0) { continue; }
                SetPixel(console, x, y, color);
            }
        }
    }

#define SPR_BODY(X, Y)                                                                                                      \
    do {                                                                                                                    \
        for (i32 py = sy; py < ey; py++, y++) {                                                                             \
            i32 xx = x;                                                                                                     \
            for (i32 px = sx; px < ex; px++, xx++) {                                                                        \
                auto col = sprr.GetPixel((X), (Y));                                                                         \
                if (!col.has_value()) { continue; }                                                                         \
                if (key.has_value()) {                                                                                      \
                    if (key.value() == col.value()) { continue; }                                                           \
                }                                                                                                           \
                SetPixel(console, xx, y, col.value());                                                                      \
            }                                                                                                               \
        }                                                                                                                   \
    } while (0);

#define REVERT(X) (SPRITE_PIX_WIDTH - 1 - (X))

    auto FrameBuffer::Spr(Console * console, i32 x, i32 y, i32 spr, i32 flip, i32 rot, std::optional<i32> key) -> void {
        auto sprr = console->GetGfx().SprData.Sprites.at(spr);
        rot &= 3;
        u32 orientation = flip & 3;

        if (rot == 1) {
            orientation ^= 1;
        } else if (rot == 2) {
            orientation ^= 3;
        } else if (rot == 3) {
            orientation ^= 2;
        }
        if (rot == 1 || rot == 3) { orientation |= 4; }

        i32 sx, sy, ex, ey;
        sx = 0;
        sy = 0;
        ex = SPRITE_PIX_WIDTH;
        ey = SPRITE_PIX_HEIGHT;
        switch (orientation) {
            case 4: SPR_BODY(py, px); break;
            case 6: SPR_BODY(REVERT(py), px); break;
            case 5: SPR_BODY(py, REVERT(px)); break;
            case 7: SPR_BODY(REVERT(py), REVERT(px)); break;
            case 0: SPR_BODY(px, py); break;
            case 2: SPR_BODY(px, REVERT(py)); break;
            case 1: SPR_BODY(REVERT(px), py); break;
            case 3: SPR_BODY(REVERT(px), REVERT(py)); break;
            default: UNREACHABLE; break;
        }
    }

#undef SPR_BODY
#undef REVERT

} // namespace dracon::console
