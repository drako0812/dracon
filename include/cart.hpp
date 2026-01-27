#pragma once

#include <istream>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include "console/font.hpp"
#include "console/pal.hpp"
#include "console/sprite.hpp"
#include "types.hpp"
#include "uid.hpp"

namespace dracon {

    static inline const std::string DefaultCode = R"(-- This is a demo hello world script
function MAIN()
puts(0, 0, "Hello from Dracon!", 23)
end
)";

    struct Author {
        char Name[32];
    };

    class Cartridge {
      public:
        Cartridge();
        static auto Load(std::istream & instrm, const std::string_view fname) -> std::variant<Cartridge, std::string>;
        static auto Load(const std::string_view file_path) -> std::variant<Cartridge, std::string>;
        auto        NewID() -> void;
        auto        Save(std::ostream & outstrm) const -> std::optional<std::string>;
        auto        Save(std::optional<const std::string_view> file_path = std::nullopt) const -> std::optional<std::string>;
        auto SaveToImage(std::optional<const std::string_view> file_path = std::nullopt) const -> std::optional<std::string>;

      public:
        UID                  ID;
        char                 ShortName[16];
        char                 LongName[64];
        Author               Authors[10];
        char                 License[32];
        char                 Version[32];

        console::FontData    Font;
        console::PaletteData Palette;
        console::SpriteData  Sprites;
        // TODO: Maps
        // TODO: Sounds
        // TODO: Songs

        i32                  Storage[256];
        std::string          Code;
    };

} // namespace dracon
