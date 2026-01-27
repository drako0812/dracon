#include "config.hpp"
#include <cerrno>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include "sago/platform_folders.h"
#include "toml++/toml.hpp"

namespace dracon {

    auto ensure_directory(const std::filesystem::path & p) {
        if (!std::filesystem::exists(p)) { std::filesystem::create_directory(p); }
    }

    auto Config::Load() -> std::variant<Config, std::string> {
        Config ret = Config{};
        ensure_directory(std::filesystem::path(sago::getConfigHome()) / "dracon");
        auto path = std::filesystem::path(sago::getConfigHome()) / "dracon" / "config.toml";
        std::cerr << std::format("Config::Load: Trying to open '{}'\n", path.string());
        if (std::filesystem::exists(path)) {
            if (std::filesystem::is_regular_file(path)) {
                std::cerr << std::format("Config::Load: '{}' successfully opened\n", path.string());
                auto config_toml = toml::parse_file(path.string());
                ret.ShowFPS      = config_toml["show_fps"].value_or(false);
            } else {
                return std::format("Config::Load: 'config.toml' exists but is not a file. '{:x}'",
                                   static_cast<int>(std::filesystem::status(path).type()));
            }
        }

        return ret;
    }

    auto Config::Save() const -> std::optional<std::string> {
        ensure_directory(std::filesystem::path(sago::getConfigHome()) / "dracon");
        auto path = std::filesystem::path(sago::getConfigHome()) / "dracon" / "config.toml";
        std::cerr << std::format("Config::Save: Trying to open '{}'\n", path.string());
        auto ofile = std::ofstream(path, std::ios::out | std::ios::trunc);
        if (!ofile) {
            std::string buf;
            buf.resize(1024);
            strerror_s(buf.data(), 1024, errno);
            return std::format("Config::Save: Unable to open '{}'\nERROR: {}", path.string(), buf);
        }

        auto tbl = toml::table{
          {"show_fps", ShowFPS},
        };

        ofile << tbl << std::endl;
        std::cerr << std::format("Config::Save: '{}' successfully saved\n", path.string());
        return std::nullopt;
    }

} // namespace dracon
