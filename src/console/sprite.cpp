#include "console/sprite.hpp"
#include <optional>
#include "console/const.hpp"
#include "console/pal.hpp"
#include "raylib.h"

namespace dracon::console {

    static auto operator==(const PaletteEntry lhs, const PaletteEntry rhs) -> bool {
        return (lhs.R == rhs.R) && (lhs.G == rhs.G) && (lhs.B == rhs.B);
    }

    static auto colorToPaletteEntry(Color col) -> PaletteEntry {
        return PaletteEntry{
          .R = col.r,
          .G = col.g,
          .B = col.b,
        };
    }

    static auto matchColorToPalette(const PaletteData & pal, Color col) -> std::optional<u8> {
        const auto colpe = colorToPaletteEntry(col);
        for (std::size_t i = 0; i < PALETTE_SIZE; i++) {
            if (colpe == pal.GetEntry(i)) { return i; }
        }
        return std::nullopt;
    }

    auto SpriteData::LoadSpriteData(const PaletteData & pal, const std::string_view file) -> bool {
        auto img = LoadImage(std::string(file).c_str());
        if (!IsImageValid(img)) { return false; }

        int sx = 0, sy = 0;
        for (std::size_t i = 0; i < SPRITE_MEMORY_SIZE; i++) {
            auto sprimg = ImageFromImage(
              img,
              Rectangle{.x = static_cast<float>(sx), .y = static_cast<float>(sy), .width = 8.0f, .height = 8.0f});

            auto & spr = Sprites.at(i);

            for (int dy = 0; dy < SPRITE_PIX_HEIGHT; dy++) {
                for (int dx = 0; dx < SPRITE_PIX_WIDTH; dx++) {
                    auto src_col = GetImageColor(sprimg, dx, dy);
                    auto src_idx = matchColorToPalette(pal, src_col);
                    if (!src_idx.has_value()) { return false; }
                    spr.SetPixel(dx, dy, src_idx.value());
                }
            }

            sx += 8;
            if (sx >= 128) {
                sx = 0;
                sy += 8;
            }
        }

        return true;
    }

    auto Sprite::SetPixel(i32 x, i32 y, i32 color) -> void {
        if ((x < 0) || (x >= SPRITE_PIX_WIDTH) || (y < 0) || (y >= SPRITE_PIX_HEIGHT)) { return; }

        std::size_t pidx = (y * SPRITE_PIX_WIDTH) + x;
        std::size_t bidx = pidx * FRAMEBUFFER_BPP;

        Pixels[bidx]     = ((color >> 4) & 1) != 0;
        Pixels[bidx + 1] = ((color >> 3) & 1) != 0;
        Pixels[bidx + 2] = ((color >> 2) & 1) != 0;
        Pixels[bidx + 3] = ((color >> 1) & 1) != 0;
        Pixels[bidx + 4] = ((color >> 0) & 1) != 0;
    }
    auto Sprite::GetPixel(i32 x, i32 y) const -> std::optional<i32> {
        if ((x < 0) || (x >= SPRITE_PIX_WIDTH) || (y < 0) || (y >= SPRITE_PIX_HEIGHT)) { return std::nullopt; }

        std::size_t pidx = (y * SPRITE_PIX_WIDTH) + x;
        std::size_t bidx = pidx * FRAMEBUFFER_BPP;
        i32         ret  = 0;
        ret              = Pixels[bidx] ? 1 : 0;
        ret              = (ret << 1) | (Pixels[bidx + 1] ? 1 : 0);
        ret              = (ret << 1) | (Pixels[bidx + 2] ? 1 : 0);
        ret              = (ret << 1) | (Pixels[bidx + 3] ? 1 : 0);
        ret              = (ret << 1) | (Pixels[bidx + 4] ? 1 : 0);
        return ret;
    }

} // namespace dracon::console
