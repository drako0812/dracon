#include "cart.hpp"
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "console/const.hpp"
#include "console/sprite.hpp"
#include "raylib.h"

#define PRINT(msg) std::cerr << (msg) << "\n";

namespace dracon {

    Cartridge::Cartridge() {
        PRINT("Cartridge: Generating ID...");
        ID = UID::Generate();

        PRINT("Cartridge: Writing ShortName");
        memset((void *)ShortName, 0, sizeof(ShortName));
        memcpy((void *)ShortName, "NEW-CART-NAME", std::strlen("NEW-CART-NAME"));

        PRINT("Cartridge: Writing LongName");
        memset((void *)LongName, 0, sizeof(LongName));
        memcpy((void *)LongName, "NEW CART LONG NAME", std::strlen("NEW CART LONG NAME"));

        PRINT("Cartridge: Writing Author #0");
        memset((void *)Authors[0].Name, 0, sizeof(Authors[0].Name));
        memcpy((void *)Authors[0].Name, "ENTER AUTHOR NAME", std::strlen("ENTER AUTHOR NAME"));

        PRINT("Cartridge: Writing other Authors");
        for (std::size_t i = 1; i < (sizeof(Authors) / sizeof(Authors[0])); i++) {
            memset((void *)Authors[i].Name, 0, sizeof(Authors[0].Name) / sizeof(char));
        }

        PRINT("Cartridge: Writing License");
        memset((void *)License, 0, sizeof(License));
        memcpy((void *)License, "ENTER LICENSE NAME", std::strlen("ENTER LICENSE NAME"));

        PRINT("Cartridge: Writing Version");
        memset((void *)Version, 0, sizeof(Version));
        memcpy((void *)Version, "0.1.0", std::strlen("0.1.0"));

        PRINT("Cartridge: Writing Storage");
        memset((void *)Storage, 0, sizeof(Storage));

        PRINT("Cartridge: Writing Code");
        Code = DefaultCode;

        PRINT("Cartridge: Writing Font");
        Font = console::FontData{};
        PRINT("Cartridge: Loading Default Font");
        Font.LoadDefault();
        PRINT("Cartridge: Writing Palette");
        Palette = console::PaletteData{};
        PRINT("Cartridge: Writing Sprites");
        Sprites = console::SpriteData{};

        PRINT("Cartridge: Success");
    }

#define TRY try {
#define ENDTRY(msg)                                                                                                         \
    }                                                                                                                       \
    catch (const std::ios_base::failure & err) {                                                                            \
        return std::format("{}{}", msg, err.what());                                                                        \
    }

    auto Cartridge::Load(std::istream & instrm, const std::string_view fname) -> std::variant<Cartridge, std::string> {
        (void)fname;
        Cartridge ret{};
        TRY;
        instrm.read((char *)(&ret.ID), sizeof(ID));
        ENDTRY("Cartridge::Load: Unable to read ID.\n");
        TRY;
        instrm.read((char *)ret.ShortName, sizeof(ret.ShortName));
        ENDTRY("Cartridge::Load: Unable to read ShortName.\n");
        TRY;
        instrm.read((char *)ret.LongName, sizeof(ret.LongName));
        ENDTRY("Cartridge::Load: Unable to read LongName.\n");
        TRY;
        instrm.read((char *)(ret.Authors), sizeof(ret.Authors));
        ENDTRY("Cartridge::Load: Unable to read Authors.\n");
        TRY;
        instrm.read((char *)ret.License, sizeof(ret.License));
        ENDTRY("Cartridge::Load: Unable to read License.\n");
        TRY;
        instrm.read((char *)ret.Version, sizeof(ret.Version));
        ENDTRY("Cartridge::Load: Unable to read Version.\n");

        TRY;
        instrm.read((char *)(&ret.Font), sizeof(ret.Font));
        ENDTRY("Cartridge::Load Unable to read Font.\n");

        TRY;
        instrm.read((char *)(&ret.Palette), sizeof(ret.Palette));
        ENDTRY("Cartridge::Load Unable to read Palette.\n");

        TRY;
        instrm.read((char *)(&ret.Sprites), sizeof(ret.Sprites));
        ENDTRY("Cartridge::Load Unable to read Sprites.\n");

        TRY;
        instrm.read((char *)(ret.Storage), sizeof(ret.Storage));
        ENDTRY("Cartridge::Load: Unable to read Storage.\n");
        std::size_t code_sz = 0;
        TRY;
        instrm.read((char *)&code_sz, sizeof(std::size_t));
        ENDTRY("Cartridge::Load: Unable to read Code Length.\n");
        ret.Code.resize(code_sz);
        TRY;
        instrm.read(ret.Code.data(), static_cast<std::streamsize>(code_sz));
        ENDTRY("Cartridge::Load: Unable to read Code.\n");
        return ret;
    }
    auto Cartridge::Load(const std::string_view file_path) -> std::variant<Cartridge, std::string> {
        std::filesystem::path p{file_path};
        p                   = p.native();
        std::ifstream ifile = std::ifstream(p, std::ios::in | std::ios::binary);
        if (!ifile) { return "Cartridge::Load: Unable to load cart file."; }

        auto ret = Load(ifile, p.string());
        ifile.close();
        return ret;
    }
    auto Cartridge::NewID() -> void { ID = UID::Generate(); }
    auto Cartridge::Save(std::ostream & outstrm) const -> std::optional<std::string> {
        TRY;
        outstrm.write((const char *)(&ID), sizeof(ID));
        ENDTRY("Cartridge::Save: Unable to write ID.\n");
        TRY;
        outstrm.write((const char *)ShortName, sizeof(ShortName));
        ENDTRY("Cartridge::Save: Unable to write ShortName.\n");
        TRY;
        outstrm.write((const char *)LongName, sizeof(LongName));
        ENDTRY("Cartridge::Save: Unable to write LongName.\n");
        TRY;
        outstrm.write((const char *)(Authors), sizeof(Authors));
        ENDTRY("Cartridge::Save: Unable to write Authors.\n");
        TRY;
        outstrm.write((const char *)License, sizeof(License));
        ENDTRY("Cartridge::Save: Unable to write License.\n");
        TRY;
        outstrm.write((const char *)Version, sizeof(Version));
        ENDTRY("Cartridge::Save: Unable to write Version.\n");

        TRY;
        outstrm.write((const char *)(&Font), sizeof(Font));
        ENDTRY("Cartridge::Save: Unable to write Font.\n");

        TRY;
        outstrm.write((const char *)(&Palette), sizeof(Palette));
        ENDTRY("Cartridge::Save: Unable to write Palette.\n");

        TRY;
        outstrm.write((const char *)(&Sprites), sizeof(Sprites));
        ENDTRY("Cartridge::Save: Unable to write Sprites.\n");

        TRY;
        outstrm.write((const char *)(&Storage), sizeof(Storage));
        ENDTRY("Cartridge::Save: Unable to write Storage.\n");
        auto sz = Code.size();
        TRY;
        outstrm.write((const char *)(&sz), sizeof(sz));
        ENDTRY("Cartridge::Save: Unable to write Code size.\n");
        TRY;
        outstrm.write(Code.data(), static_cast<std::streamsize>(sz));
        ENDTRY("Cartridge::Save: Unable to write Code.\n");
        return std::nullopt;
    }
    auto Cartridge::Save(std::optional<const std::string_view> file_path) const -> std::optional<std::string> {
        std::filesystem::path p{file_path.value_or(std::format("{}.dccart", ShortName))};
        p                   = p.native();
        std::ofstream ofile = std::ofstream(p, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!ofile) { return "Cartridge::Save: Unable to open cart file."; }

        auto ret = Save(ofile);
        ofile.close();
        return ret;
    }

    static auto SplitToNibbles(const std::string_view source) -> std::vector<u8> {
        std::vector<u8> ret{};
        for (auto ch : source) {
            ret.push_back((ch >> 4) & 0xF);
            ret.push_back(ch & 0xF);
        }
        return ret;
    }

    static auto GroupNibblesBy4(const std::vector<u8> & source) -> std::vector<std::array<u8, 4>> {
        std::vector<std::array<u8, 4>> ret{};
        for (std::size_t i = 0; i < source.size(); i += 4) {
            std::array<u8, 4> arr = {0, 0, 0, 0};
            arr.at(0)             = source.at(i);
            if ((i + 2) < source.size()) { arr.at(1) = source.at(i + 1); }
            if ((i + 3) < source.size()) { arr.at(2) = source.at(i + 2); }
            if ((i + 4) < source.size()) { arr.at(3) = source.at(i + 3); }
            ret.push_back(arr);
        }
        return ret;
    }

    constexpr std::size_t MAX_IMAGE_CART_SIZE = ((console::FRAMEBUFFER_PIX_WIDTH * console::FRAMEBUFFER_PIX_HEIGHT) * 4) / 2;

    auto Cartridge::SaveToImage(std::optional<const std::string_view> file_path) const -> std::optional<std::string> {
        std::ostringstream os = std::ostringstream(std::ios::out | std::ios::binary | std::ios::trunc);

        auto               ret = Save(os);
        if (ret.has_value()) { return ret; }
        auto s = os.str();

        if (s.length() >= MAX_IMAGE_CART_SIZE) {
            return std::format("Cartridge::SaveToImage: Cartridge data is too large, got {}, max {}",
                               s.length(),
                               MAX_IMAGE_CART_SIZE);
        }

        auto data = GroupNibblesBy4(SplitToNibbles(s));

        auto img = GenImageColor(console::FRAMEBUFFER_PIX_WIDTH, console::FRAMEBUFFER_PIX_HEIGHT, BLACK);

        int  x = 0;
        int  y = 0;
        for (std::size_t i = 0; (i < (static_cast<std::size_t>(img.width) * img.height)) && (i < data.size()); i++) {
            auto         color       = GetImageColor(img, x, y);
            const auto & datum       = data.at(i);
            constexpr u8 HIGH_NIBBLE = 0xF0;
            color.r                  = (color.r & HIGH_NIBBLE) | datum.at(0);
            color.g                  = (color.g & HIGH_NIBBLE) | datum.at(1);
            color.b                  = (color.b & HIGH_NIBBLE) | datum.at(2);
            color.a                  = (color.a & HIGH_NIBBLE) | datum.at(3);
            ImageDrawPixel(&img, x, y, color);

            x++;
            if (x >= console::FRAMEBUFFER_PIX_WIDTH) {
                x = 0;
                y++;
            }
        }

        std::filesystem::path p{file_path.value_or(std::format("{}-cart.png", ShortName))};
        p = p.native();

        if (!ExportImage(img, p.string().c_str())) {
            return std::format("Cartridge::SaveToImage: Unable to save image {}", p.string());
        }

        return std::nullopt;
    }

} // namespace dracon
