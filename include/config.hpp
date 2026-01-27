#pragma once

#include <optional>
#include <string>
#include <variant>

namespace dracon {

    class Config {
      public:
        static auto Load() -> std::variant<Config, std::string>;
        auto        Save() const -> std::optional<std::string>;

      public:
        bool ShowFPS = false;
    };

} // namespace dracon
