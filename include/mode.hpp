#pragma once
#include <string>
#include "types.hpp"

namespace dracon {

    class Console;

    enum class ConsoleMode : u8 {
        Prompt,
        Editor,
        Player,
    };

    class Mode {
      public:
        Mode()                                           = default;
        Mode(const Mode & other)                         = default;
        Mode(Mode && other) noexcept                     = default;
        auto operator=(const Mode & other) -> Mode &     = default;
        auto operator=(Mode && other) noexcept -> Mode & = default;
        virtual ~Mode();
        virtual auto OnLoad(Console * console) -> void          = 0;
        virtual auto OnUnload(Console * console) -> void        = 0;
        virtual auto OnUpdate(Console * console) -> void        = 0;
        virtual auto OnPreRender(Console * console) -> void     = 0;
        virtual auto OnRender(Console * console) -> void        = 0;
        virtual auto GetModeName() const -> const std::string & = 0;
    };

} // namespace dracon
