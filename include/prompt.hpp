#pragma once
#include <array>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "console.hpp"
#include "console/const.hpp"
#include "console/pal.hpp"
#include "mode.hpp"
#include "types.hpp"

namespace dracon {

    struct Cell {
        u8   Fg;
        u8   Bg;
        char Ch;
    };

    class Terminal {
      public:
        static constexpr std::size_t Width            = console::FRAMEBUFFER_PIX_WIDTH / console::SPRITE_PIX_WIDTH;
        static constexpr std::size_t Height           = console::FRAMEBUFFER_PIX_HEIGHT / console::SPRITE_PIX_HEIGHT;
        static constexpr std::size_t VisibleCharCount = Width * Height;

        Terminal();
        auto               Cls() -> void;
        auto               PutChar(int col, std::size_t line, u8 fg, u8 bg, char ch) -> void;
        auto               PrintChar(u8 fg, u8 bg, char ch) -> void;
        auto               PrintChar(char ch) -> void;
        auto               PrintString(u8 fg, u8 bg, const std::string_view s) -> void;
        auto               PrintString(const std::string_view s) -> void;
        auto               ClearLine(std::size_t line) -> void;
        auto               UnprintChar() -> void;
        auto               ScrollToLine(std::size_t line) -> void;
        auto               SetCursorLine(std::size_t line) -> void;
        auto               SetCursorColumn(int col) -> void;
        auto               SetCursorPosition(int col, std::size_t line) -> void;
        auto               SetCursorVisible(bool visible) -> void;
        auto               SetCursorChar(char ch) -> void;
        auto               SetCursorFg(u8 fg) -> void;
        auto               SetCursorInfo(u8 fg, char ch) -> void;
        auto               SetCurrentFg(u8 fg) -> void;
        auto               SetCurrentBg(u8 bg) -> void;
        auto               SetCurrentColor(u8 fg, u8 bg) -> void;
        auto               SetDefaultChar(u8 fg, u8 bg, char ch) -> void;
        auto               SetInterpretCtrlChars(bool enabled) -> void;
        auto               Reset() -> void;
        auto               Render(Console *console) -> void;
        [[nodiscard]] auto GetChar(int col, std::size_t line) const -> std::optional<Cell>;
        [[nodiscard]] auto GetScrollLine() const -> std::size_t;
        [[nodiscard]] auto GetCursorLine() const -> std::size_t;
        [[nodiscard]] auto GetCursorColumn() const -> int;
        [[nodiscard]] auto GetCursorPosition() const -> std::pair<int, std::size_t>;
        [[nodiscard]] auto GetCursorVisible() const -> bool;
        [[nodiscard]] auto GetCursorChar() const -> char;
        [[nodiscard]] auto GetCursorFg() const -> u8;
        [[nodiscard]] auto GetCursorInfo() const -> std::pair<u8, char>;
        [[nodiscard]] auto GetCurrentFg() const -> u8;
        [[nodiscard]] auto GetCurrentBg() const -> u8;
        [[nodiscard]] auto GetCurrentColor() const -> std::pair<u8, u8>;
        [[nodiscard]] auto GetDefaultChar() const -> Cell;
        [[nodiscard]] auto GetInterpretCtrlChars() const -> bool;

      private:
        auto                                 ensureLines(std::size_t line_needed) -> void;

        std::vector<std::array<Cell, Width>> m_lines;
        std::size_t                          m_vscroll;
        std::size_t                          m_csr_line;
        u8                                   m_csr_column;
        bool                                 m_csr_visible;
        char                                 m_csr_ch;
        u8                                   m_csr_fg;
        u8                                   m_cur_fg;
        u8                                   m_cur_bg;
        bool                                 m_interpret_ctrl_chars;
        Cell                                 m_default_ch;
    };

    class Argument {
      public:
        Argument() = delete;
        Argument(const std::string_view name, const std::optional<std::string_view> value = std::nullopt);
        [[nodiscard]] auto GetName() const -> std::string;
        [[nodiscard]] auto Value() const -> std::string;

        /// @brief Returns the value as a `bool` if possible
        /// @return `true` if `Value` is empty;
        ///         `true` if `Value` is some form of `true`;
        ///         `true` if `Value` is `1`;
        ///         `false` if `Value` is some form of `false`;
        ///         `false` if `Value` is `0`;
        ///         `nullopt` otherwise
        [[nodiscard]] auto ValueAsBool() const -> std::optional<bool>;

        /// @brief Returns the value as an `i32` if possible
        /// @return Actual value if `Value` is convertible to an i32;
        ///         `1` if `Value` is empty;
        ///         `nullopt` otherwise
        [[nodiscard]] auto ValueAsInt() const -> std::optional<i32>;

      private:
        std::string                m_name;
        std::optional<std::string> m_value;
    };

    class Command {
      public:
        Command() = delete;
        Command(const std::span<Argument> args);
        virtual ~Command()                               = default;
        virtual auto Run(Console *console) -> i32        = 0;
        virtual auto Help() const -> const std::string & = 0;

      protected:
        std::vector<Argument> m_args;
    };

    class EchoCommand : public Command {
      public:
        EchoCommand() = delete;
        EchoCommand(const std::span<Argument> args);
        ~EchoCommand() override;
        auto Run(Console *console) -> i32 override;
        auto Help() const -> const std::string & override;
    };

    class HelpCommand : public Command {
      public:
        HelpCommand() = delete;
        HelpCommand(const std::span<Argument> args);
        ~HelpCommand() override;
        auto Run(Console *console) -> i32 override;
        auto Help() const -> const std::string & override;
    };

    class ExitCommand : public Command {
      public:
        ExitCommand() = delete;
        ExitCommand(const std::span<Argument> args);
        ~ExitCommand() override;
        auto Run(Console *console) -> i32 override;
        auto Help() const -> const std::string & override;
    };

    class Prompt : public Mode {
      public:
        Prompt();
        Prompt(const std::string_view starting_msg, u8 fg = console::pal::White, u8 bg = console::pal::Black);
        Prompt(const Prompt &other);
        Prompt(Prompt &&other) noexcept;
        auto operator=(const Prompt &other) -> Prompt &;
        auto operator=(Prompt &&other) noexcept -> Prompt &;
        ~Prompt() override;
        auto OnLoad(Console *console) -> void override;
        auto OnUnload(Console *console) -> void override;
        auto OnUpdate(Console *console) -> void override;
        auto OnPreRender(Console *console) -> void override;
        auto OnRender(Console *console) -> void override;
        auto GetModeName() const -> const std::string & override;
        auto GetTerminal() -> Terminal *;

      private:
        auto                     interpretCommand(Console *console, const std::string_view line) -> i32;
        auto                     parseCommand(Console *console, const std::string_view line) -> sptr<Command>;

        Terminal                 m_term;
        std::string              m_command_line;
        std::vector<std::string> m_history;
        std::size_t              m_history_idx;
    };

} // namespace dracon
