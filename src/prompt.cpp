#include <array>
#include <cctype>
#include <format>
#include <iterator>
#include <map>
#include <memory>
#include <mode.hpp>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <types.hpp>
#include <util.hpp>
#include <utility>
#include <vector>
#include "console.hpp"
#include "console/const.hpp"
#include "console/pal.hpp"
#include "prompt.hpp"
#include "raylib.h"

namespace dracon {

    Terminal::Terminal() :
      m_lines{},
      m_vscroll{0},
      m_csr_line{0},
      m_csr_column{0},
      m_csr_visible{true},
      m_csr_ch{'_'},
      m_csr_fg{console::pal::White},
      m_cur_fg{console::pal::White},
      m_cur_bg{console::pal::Black},
      m_interpret_ctrl_chars{true},
      m_default_ch{Cell{.Fg = console::pal::White, .Bg = console::pal::Black, .Ch = ' '}} {
        Cls();
    }

    auto Terminal::Reset() -> void {
        m_lines                = {};
        m_vscroll              = 0;
        m_csr_line             = 0;
        m_csr_column           = 0;
        m_csr_visible          = true;
        m_csr_ch               = '_';
        m_csr_fg               = console::pal::White;
        m_cur_fg               = console::pal::White;
        m_cur_bg               = console::pal::Black;
        m_interpret_ctrl_chars = true;
        m_default_ch           = Cell{.Fg = console::pal::White, .Bg = console::pal::Black, .Ch = ' '};
        Cls();
    }
    auto Terminal::Cls() -> void {
        m_lines.resize(Height);
        for (auto &line : m_lines) {
            line = {};
            for (auto &c : line) { c = m_default_ch; }
        }
        m_vscroll    = 0;
        m_csr_line   = 0;
        m_csr_column = 0;
    }

    auto Terminal::ensureLines(std::size_t line_needed) -> void {
        while (m_lines.size() < (line_needed + 1)) {
            std::array<Cell, Width> line = {};
            for (auto &c : line) { c = m_default_ch; }
            m_lines.push_back(line);
        }
    }

    auto Terminal::PutChar(int col, std::size_t line, u8 fg, u8 bg, char ch) -> void {
        ensureLines(line);

        m_lines.at(line).at(col) = Cell{.Fg = fg, .Bg = bg, .Ch = ch};
    }
    auto Terminal::PrintChar(u8 fg, u8 bg, char ch) -> void {
        if (!m_interpret_ctrl_chars) {
            PutChar(m_csr_column, m_csr_line, fg, bg, ch);
            m_csr_column++;
            if (m_csr_column >= Width) {
                m_csr_column = 0;
                m_csr_line++;
                ensureLines(m_csr_line);
            }
            ScrollToLine(m_csr_line);
            return;
        }

        switch (ch) {
            case '\n':
                m_csr_column = 0;
                m_csr_line++;
                ensureLines(m_csr_line);
                break;
            case '\r': m_csr_column = 0; break;
            case '\t':
                if (m_csr_column < Width - 2) {
                    m_csr_column = (m_csr_column + 2) & ~(2 - 1);
                } else {
                    m_csr_column = 0;
                    m_csr_line++;
                    ensureLines(m_csr_line);
                }
                break;
            case '\b':
                if (m_csr_column > 0) { m_csr_column--; }
                break;
            default:
                PutChar(m_csr_column, m_csr_line, fg, bg, ch);
                m_csr_column++;
                if (m_csr_column >= Width) {
                    m_csr_column = 0;
                    m_csr_line++;
                    ensureLines(m_csr_line);
                }
                break;
        }
        ScrollToLine(m_csr_line);
    }
    auto Terminal::PrintChar(char ch) -> void { PrintChar(m_cur_fg, m_cur_bg, ch); }
    auto Terminal::PrintString(u8 fg, u8 bg, const std::string_view s) -> void {
        for (auto c : s) { PrintChar(fg, bg, c); }
    }
    auto Terminal::PrintString(const std::string_view s) -> void { PrintString(m_cur_fg, m_cur_bg, s); }

    auto Terminal::ClearLine(std::size_t line) -> void {
        ensureLines(line);
        auto &line_str = m_lines.at(line);
        for (auto &c : line_str) { c = m_default_ch; }
    }

    auto Terminal::UnprintChar() -> void {
        if (m_csr_column > 0) {
            m_csr_column--;
        } else {
            if (m_csr_line > 0) {
                m_csr_line--;
                m_csr_column = Width - 1;
            }
        }
        PutChar(m_csr_column, m_csr_line, m_default_ch.Fg, m_default_ch.Bg, m_default_ch.Ch);
    }

    auto Terminal::ScrollToLine(std::size_t line) -> void {
        // The algorithm we will be using will attempt to move the scrolling window the least
        // to get the passed `line` on screen, as long as that line exists, otherwise it moves as close
        // as possible.
        std::size_t cur_max_line = m_vscroll + (Height)-1;
        if (line > cur_max_line) {
            // We need to scroll down
            while ((line > cur_max_line) && (line < m_lines.size())) {
                m_vscroll++;
                cur_max_line = m_vscroll + (Height)-1;
            }
        } else if (line < m_vscroll) {
            // We need to scroll up
            while ((line < m_vscroll) && (line >= 0)) {
                m_vscroll--;
                // cur_max_line = m_vscroll + (Height)-1;
            }
        }

        SetWindowTitle(std::format("dracon | [m_vscroll={}", m_vscroll).c_str());

        // TODO: Optimize this. It should be able to calculate this without looping.
    }
    auto Terminal::SetCursorLine(std::size_t line) -> void { m_csr_line = line; }
    auto Terminal::SetCursorColumn(int col) -> void { m_csr_column = col; }
    auto Terminal::SetCursorPosition(int col, std::size_t line) -> void {
        m_csr_line   = line;
        m_csr_column = col;
    }
    auto Terminal::SetCursorVisible(bool visible) -> void { m_csr_visible = visible; }
    auto Terminal::SetCursorChar(char ch) -> void { m_csr_ch = ch; }
    auto Terminal::SetCursorFg(u8 fg) -> void { m_csr_fg = fg; }
    auto Terminal::SetCursorInfo(u8 fg, char ch) -> void {
        m_csr_fg = fg;
        m_csr_ch = ch;
    }
    auto Terminal::SetCurrentFg(u8 fg) -> void { m_cur_fg = fg; }
    auto Terminal::SetCurrentBg(u8 bg) -> void { m_cur_bg = bg; }
    auto Terminal::SetCurrentColor(u8 fg, u8 bg) -> void {
        m_cur_fg = fg;
        m_cur_bg = bg;
    }
    auto Terminal::SetDefaultChar(u8 fg, u8 bg, char ch) -> void { m_default_ch = Cell{.Fg = fg, .Bg = bg, .Ch = ch}; }
    auto Terminal::GetChar(int col, std::size_t line) const -> std::optional<Cell> {
        if ((col < 0) || (static_cast<std::size_t>(col) >= Width) || (line >= m_lines.size())) { return std::nullopt; }

        const auto &linec = m_lines.at(line);
        return linec.at(col);
    }
    auto Terminal::GetScrollLine() const -> std::size_t { return m_vscroll; }
    auto Terminal::GetCursorLine() const -> std::size_t { return m_csr_line; }
    auto Terminal::GetCursorColumn() const -> int { return m_csr_column; }
    auto Terminal::GetCursorPosition() const -> std::pair<int, std::size_t> {
        return std::make_pair(m_csr_column, m_csr_line);
    }
    auto Terminal::GetCursorVisible() const -> bool { return m_csr_visible; }
    auto Terminal::GetCursorChar() const -> char { return m_csr_ch; }
    auto Terminal::GetCursorFg() const -> u8 { return m_csr_fg; }
    auto Terminal::GetCursorInfo() const -> std::pair<u8, char> { return std::make_pair(m_csr_fg, m_csr_ch); }
    auto Terminal::GetCurrentFg() const -> u8 { return m_cur_fg; }
    auto Terminal::GetCurrentBg() const -> u8 { return m_cur_bg; }
    auto Terminal::GetCurrentColor() const -> std::pair<u8, u8> { return std::make_pair(m_cur_fg, m_cur_bg); }
    auto Terminal::GetDefaultChar() const -> Cell { return m_default_ch; }
    auto Terminal::SetInterpretCtrlChars(bool enabled) -> void { m_interpret_ctrl_chars = enabled; }
    auto Terminal::GetInterpretCtrlChars() const -> bool { return m_interpret_ctrl_chars; }
    auto Terminal::Render(Console *console) -> void {
        for (std::size_t line = m_vscroll; line < (m_vscroll + Height); line++) {
            for (int column = 0; static_cast<std::size_t>(column) < Width; column++) {
                auto cell = GetChar(column, line).value_or(m_default_ch);
                console->GetGfx().FrameBuf.RectF(console,
                                                 static_cast<i32>(column * console::SPRITE_PIX_WIDTH),
                                                 static_cast<i32>((line - m_vscroll) * console::SPRITE_PIX_HEIGHT),
                                                 console::SPRITE_PIX_WIDTH,
                                                 console::SPRITE_PIX_HEIGHT,
                                                 cell.Bg);
                console->GetGfx().FrameBuf.PutCh(console,
                                                 static_cast<i32>(column * console::SPRITE_PIX_WIDTH),
                                                 static_cast<i32>((line - m_vscroll) * console::SPRITE_PIX_HEIGHT),
                                                 cell.Ch,
                                                 cell.Fg);
                if (m_csr_visible && (line == m_csr_line) && (column == static_cast<int>(m_csr_column))) {
                    console->GetGfx().FrameBuf.PutCh(console,
                                                     static_cast<i32>(column * console::SPRITE_PIX_WIDTH),
                                                     static_cast<i32>((line - m_vscroll) * console::SPRITE_PIX_HEIGHT),
                                                     m_csr_ch,
                                                     m_csr_fg);
                }
            }
        }
    }

    Prompt::Prompt() : Mode{}, m_term{}, m_command_line{}, m_history{}, m_history_idx{0} { }
    Prompt::Prompt(const std::string_view starting_msg, u8 fg, u8 bg) : Prompt{} {
        m_term.PrintString(fg, bg, starting_msg);
    }
    Prompt::Prompt(const Prompt &other)                         = default;
    Prompt::Prompt(Prompt &&other) noexcept                     = default;
    auto Prompt::operator=(const Prompt &other) -> Prompt &     = default;
    auto Prompt::operator=(Prompt &&other) noexcept -> Prompt & = default;
    Prompt::~Prompt() { }
    auto                                     Prompt::OnLoad(Console *console) -> void { (void)console; }
    auto                                     Prompt::OnUnload(Console *console) -> void { }

    static const std::map<KeyboardKey, char> key2char = {
      {KEY_APOSTROPHE, '\''}, {KEY_COMMA, ','},
      {KEY_MINUS, '-'},       {KEY_PERIOD, '.'},
      {KEY_SLASH, '/'},       {KEY_ZERO, '0'},
      {KEY_ONE, '1'},         {KEY_TWO, '2'},
      {KEY_THREE, '3'},       {KEY_FOUR, '4'},
      {KEY_FIVE, '5'},        {KEY_SIX, '6'},
      {KEY_SEVEN, '7'},       {KEY_EIGHT, '8'},
      {KEY_NINE, '9'},        {KEY_SEMICOLON, ';'},
      {KEY_EQUAL, '='},       {KEY_A, 'a'},
      {KEY_B, 'b'},           {KEY_C, 'c'},
      {KEY_D, 'd'},           {KEY_E, 'e'},
      {KEY_F, 'f'},           {KEY_G, 'g'},
      {KEY_H, 'h'},           {KEY_I, 'i'},
      {KEY_J, 'j'},           {KEY_K, 'k'},
      {KEY_L, 'l'},           {KEY_M, 'm'},
      {KEY_N, 'n'},           {KEY_O, 'o'},
      {KEY_P, 'p'},           {KEY_Q, 'q'},
      {KEY_R, 'r'},           {KEY_S, 's'},
      {KEY_T, 't'},           {KEY_U, 'u'},
      {KEY_V, 'v'},           {KEY_W, 'w'},
      {KEY_X, 'x'},           {KEY_Y, 'y'},
      {KEY_Z, 'z'},           {KEY_LEFT_BRACKET, '['},
      {KEY_BACKSLASH, '\\'},  {KEY_RIGHT_BRACKET, ']'},
      {KEY_GRAVE, '`'},       {KEY_SPACE, ' '},
      {KEY_ENTER, '\n'},      {KEY_TAB, '\t'},
      {KEY_BACKSPACE, '\b'},  {KEY_KP_0, '0'},
      {KEY_KP_1, '1'},        {KEY_KP_2, '2'},
      {KEY_KP_3, '3'},        {KEY_KP_4, '4'},
      {KEY_KP_5, '5'},        {KEY_KP_6, '6'},
      {KEY_KP_7, '7'},        {KEY_KP_8, '8'},
      {KEY_KP_9, '9'},        {KEY_KP_DECIMAL, '.'},
      {KEY_KP_DIVIDE, '/'},   {KEY_KP_MULTIPLY, '*'},
      {KEY_KP_SUBTRACT, '-'}, {KEY_KP_ADD, '+'},
      {KEY_KP_ENTER, '\n'},   {KEY_KP_EQUAL, '='},
    };

    static const std::map<KeyboardKey, char> key2char_shift = {
      {KEY_APOSTROPHE, '"'}, {KEY_COMMA, '<'},
      {KEY_MINUS, '_'},      {KEY_PERIOD, '>'},
      {KEY_SLASH, '?'},      {KEY_ZERO, ')'},
      {KEY_ONE, '!'},        {KEY_TWO, '@'},
      {KEY_THREE, '#'},      {KEY_FOUR, '$'},
      {KEY_FIVE, '%'},       {KEY_SIX, '^'},
      {KEY_SEVEN, '&'},      {KEY_EIGHT, '*'},
      {KEY_NINE, '('},       {KEY_SEMICOLON, ':'},
      {KEY_EQUAL, '+'},      {KEY_A, 'A'},
      {KEY_B, 'B'},          {KEY_C, 'C'},
      {KEY_D, 'D'},          {KEY_E, 'E'},
      {KEY_F, 'F'},          {KEY_G, 'G'},
      {KEY_H, 'H'},          {KEY_I, 'I'},
      {KEY_J, 'J'},          {KEY_K, 'K'},
      {KEY_L, 'L'},          {KEY_M, 'M'},
      {KEY_N, 'N'},          {KEY_O, 'O'},
      {KEY_P, 'P'},          {KEY_Q, 'Q'},
      {KEY_R, 'R'},          {KEY_S, 'S'},
      {KEY_T, 'T'},          {KEY_U, 'U'},
      {KEY_V, 'V'},          {KEY_W, 'W'},
      {KEY_X, 'X'},          {KEY_Y, 'Y'},
      {KEY_Z, 'Z'},          {KEY_LEFT_BRACKET, '{'},
      {KEY_BACKSLASH, '|'},  {KEY_RIGHT_BRACKET, '}'},
      {KEY_GRAVE, '~'},      {KEY_SPACE, ' '},
      /*{KEY_ENTER, '\n'},     {KEY_TAB, '\t'},
      {KEY_BACKSPACE, '\b'},  {KEY_KP_0, '0'},
      {KEY_KP_1, '1'},        {KEY_KP_2, '2'},
      {KEY_KP_3, '3'},        {KEY_KP_4, '4'},
      {KEY_KP_5, '5'},        {KEY_KP_6, '6'},
      {KEY_KP_7, '7'},        {KEY_KP_8, '8'},
      {KEY_KP_9, '9'},        {KEY_KP_DECIMAL, '.'},
      {KEY_KP_DIVIDE, '/'},   {KEY_KP_MULTIPLY, '*'},
      {KEY_KP_SUBTRACT, '-'}, {KEY_KP_ADD, '+'},
      {KEY_KP_ENTER, '\n'},   {KEY_KP_EQUAL, '='},*/
    };

    static auto tolower(const std::string_view source) -> std::string {
        auto new_string = (source | std::views::transform([](u8 c) { return std::tolower(c); }));
        return {new_string.begin(), new_string.end()};
    }

    auto Prompt::OnUpdate(Console *console) -> void {
        // TODO: Make command line editing better. Ideally, we would be able to use arrow keys to move around the cursor,
        // which means we'd also need to keep track of the current position in the command_line string.

        auto key = GetKeyPressed();
        while (key != 0) {
            if ((key == KEY_ENTER) || (key == KEY_KP_ENTER)) {
                // Submit command line
                m_term.PrintChar('\n');

                if (!m_command_line.empty()) {
                    m_history.push_back(m_command_line);
                    m_history_idx = m_history.size();
                }
                while (m_history.size() > 100) { m_history = {std::begin(m_history)++, std::end(m_history)}; }
                auto cmd_ret = interpretCommand(console, m_command_line);
                if (cmd_ret != 0) {
                    m_term.PrintString(console::pal::Red, console::pal::Black, std::format("{}\xff", cmd_ret));
                } else {
                    m_term.PrintChar('\xff');
                }
                m_command_line = "";
            } else if (key == KEY_UP) {
                if (m_history.size() > 0) {
                    if (m_history_idx > 0) { m_history_idx--; }
                    m_term.ClearLine(m_term.GetCursorLine());
                    m_term.SetCursorColumn(0);
                    m_term.PrintChar('\xff');
                    m_term.PrintString(m_history.at(m_history_idx));
                    m_command_line = m_history.at(m_history_idx);
                }
            } else if (key == KEY_DOWN) {
                m_history_idx++;
                if (m_history_idx > m_history.size()) { m_history_idx = m_history.size(); }
                if (m_history_idx == m_history.size()) {
                    m_term.ClearLine(m_term.GetCursorLine());
                    m_term.SetCursorColumn(0);
                    m_term.PrintChar('\xff');
                    m_command_line = "";
                } else {
                    m_term.ClearLine(m_term.GetCursorLine());
                    m_term.SetCursorColumn(0);
                    m_term.PrintChar('\xff');
                    m_term.PrintString(m_history.at(m_history_idx));
                    m_command_line = m_history.at(m_history_idx);
                }
            } else {
                if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                    auto iter = key2char_shift.find(static_cast<KeyboardKey>(key));
                    if (iter != key2char_shift.cend()) {
                        if (iter->second != '\b') {
                            m_term.PrintChar(iter->second);
                            m_command_line.push_back(iter->second);
                        } else {
                            if (!m_command_line.empty()) {
                                // m_term.PrintChar('\b');
                                m_term.UnprintChar();
                                m_command_line.pop_back();
                            }
                        }
                    }
                } else {
                    auto iter = key2char.find(static_cast<KeyboardKey>(key));
                    if (iter != key2char.cend()) {
                        if (iter->second != '\b') {
                            m_term.PrintChar(iter->second);
                            m_command_line.push_back(iter->second);
                        } else {
                            if (!m_command_line.empty()) {
                                // m_term.PrintChar('\b');
                                m_term.UnprintChar();
                                m_command_line.pop_back();
                            }
                        }
                    }
                }
            }
            key = GetKeyPressed();
        }

        auto mmove = GetMouseWheelMoveV();
        if (mmove.y > 0) {
            auto line = m_term.GetScrollLine();
            line--;
            m_term.ScrollToLine(line);
        } else if (mmove.y < 0) {
            auto line = m_term.GetScrollLine();
            line += Terminal::Height;
            m_term.ScrollToLine(line);
        }
    }
    auto Prompt::OnPreRender(Console *console) -> void { m_term.Render(console); }
    auto Prompt::OnRender(Console *console) -> void { (void)console; }
    auto Prompt::GetModeName() const -> const std::string & {
        static const std::string name = "prompt";
        return name;
    }
    auto Prompt::GetTerminal() -> Terminal * { return &m_term; }

    auto Prompt::interpretCommand(Console *console, const std::string_view line) -> i32 {
        if (line.empty()) { return 0; }
        auto cmd = parseCommand(console, line);
        if (cmd != nullptr) {
            return cmd->Run(console);
        } else {
            return -0xDEADBEEF;
        }
    }

#define EAT_WHITESPACE(iter, end_iter, end_label)                                                                           \
    while (std::isspace(*(iter))) {                                                                                         \
        (iter)++;                                                                                                           \
        if ((iter) == (end_iter)) { goto end_label; }                                                                       \
    }

    auto Prompt::parseCommand(Console *console, const std::string_view line) -> sptr<Command> {
        auto                     iter         = line.cbegin();
        std::string              command_name = "";
        std::vector<std::string> args         = {};

        if (line.empty()) { return nullptr; }

        // Get command name first
        while (!std::isspace(*iter)) {
            command_name.push_back(*iter);
            iter++;
            if (iter == line.cend()) { goto end; }
        }

        EAT_WHITESPACE(iter, line.cend(), end);

        // Parse arguments
        for (;;) {
            if (*iter == '\"') {
                // Parse quoted string
                std::string arg = "";
                iter++;
                if (iter == line.cend()) {
                    arg.push_back('\"');
                    args.push_back(arg);
                    goto end;
                }

                while (*iter != '\"') {
                    if (*iter == '\\') {
                        // Read an escape character, currently just `"` and `\`
                        iter++;
                        if (iter == line.cend()) {
                            arg.push_back('\\');
                            args.push_back(arg);
                            goto end;
                        }
                        if (*iter == '\"') {
                            arg.push_back('\"');
                        } else if (*iter == '\\') {
                            arg.push_back('\\');
                        }
                        iter++;
                        if (iter == line.cend()) {
                            args.push_back(arg);
                            goto end;
                        }
                    } else {
                        arg.push_back(*iter);
                        iter++;
                        if (iter == line.cend()) {
                            args.push_back(arg);
                            goto end;
                        }
                    }
                }
                args.push_back(arg);

                iter++;
                if (iter == line.cend()) { goto end; }

                EAT_WHITESPACE(iter, line.cend(), end);
            } else {
                // Unquoted string
                std::string arg = "";
                while (!std::isspace(*iter)) {
                    arg.push_back(*iter);
                    iter++;
                    if (iter == line.cend()) {
                        args.push_back(arg);
                        goto end;
                    }
                }

                args.push_back(arg);

                EAT_WHITESPACE(iter, line.cend(), end);
            }
        }

    end:
        // Now we need to parse the argument strings to split the keys and values.
        std::vector<Argument> args_final = {};
        for (auto const &arg : args) {
            auto pos = arg.find_first_of('=');
            if (pos == std::string::npos) {
                // No equals sign
                auto a = Argument(arg);
                args_final.push_back(a);
            } else {
                auto key   = arg.substr(0, pos);
                auto value = arg.substr(pos + 1);
                auto a     = Argument(key, value);
                args_final.push_back(a);
            }
        }

        command_name = tolower(command_name);

        if (command_name == "echo") {
            return std::make_shared<EchoCommand>(args_final);
        } else if (command_name == "help") {
            return std::make_shared<HelpCommand>(args_final);
        } else if (command_name == "exit") {
            return std::make_shared<ExitCommand>(args_final);
        } else {
            GetTerminal()->PrintString(console::pal::Red,
                                       console::pal::Black,
                                       std::format("ERROR: Unknown Command `{}`\n", command_name));
            return nullptr;
        }
    }

    Argument::Argument(const std::string_view name, const std::optional<std::string_view> value) :
      m_name{name},
      m_value{value} { }

    auto Argument::GetName() const -> std::string { return m_name; }

    auto Argument::Value() const -> std::string { return m_value.value_or(m_name); }

    auto Argument::ValueAsBool() const -> std::optional<bool> {
        if (m_value.has_value()) {
            if (tolower(m_value.value()) == "true") {
                return true;
            } else if (m_value.value() == "1") {
                return true;
            } else if (tolower(m_value.value()) == "false") {
                return false;
            } else if (m_value.value() == "0") {
                return false;
            } else {
                return std::nullopt;
            }
        } else {
            return true;
        }
    }

    auto Argument::ValueAsInt() const -> std::optional<i32> {
        if (m_value.has_value()) {
            try {
                i64 ret = std::stoll(m_value.value(), nullptr, 0);
                return static_cast<i32>(ret);
            } catch (std::invalid_argument &err) { return std::nullopt; } catch (std::out_of_range &err) {
                return std::nullopt;
            }
            UNREACHABLE;
        } else {
            return 1;
        }
    }

    Command::Command(const std::span<Argument> args) : m_args(args.begin(), args.end()) { }

    EchoCommand::EchoCommand(const std::span<Argument> args) : Command{args} { }

    EchoCommand::~EchoCommand() { }

    auto EchoCommand::Run(Console *console) -> i32 {
        auto mode = console->GetModeInstance();
        if (mode->GetModeName() != "prompt") { return -1; }
        auto prompt = dynamic_cast<Prompt *>(mode);
        auto term   = prompt->GetTerminal();
        for (auto &arg : m_args) {
            auto v = arg.Value();
            term->PrintString(console::pal::LightGray, console::pal::Black, std::format("{}\n", v));
        }
        return 0;
    }

    auto EchoCommand::Help() const -> const std::string & {
        static const std::string help_str = R"(echo
  Prints a newline to the console.

echo <msg>
  Prints the msg to the console.

echo <msg> [msg...]
  Prints each msg to the console separated by a newline.
)";
        return help_str;
    }

    HelpCommand::HelpCommand(const std::span<Argument> args) : Command{args} { }

    HelpCommand::~HelpCommand() { }

    auto HelpCommand::Run(Console *console) -> i32 {
        auto mode = console->GetModeInstance();
        if (mode->GetModeName() != "prompt") { return -1; }
        auto prompt = dynamic_cast<Prompt *>(mode);
        auto term   = prompt->GetTerminal();

        if (m_args.size() > 0) {
            auto arg = m_args.at(0).Value();
            if (arg == "echo") {
                std::vector<Argument> argz = {};
                auto                  c    = std::make_shared<EchoCommand>(argz);
                term->PrintString(console::pal::LightGray, console::pal::Black, c->Help());
                return 0;
            } else if (arg == "help") {
                term->PrintString(Help());
                return 0;
            } else if (arg == "exit") {
                std::vector<Argument> argz = {};
                auto                  c    = std::make_shared<ExitCommand>(argz);
                term->PrintString(console::pal::LightGray, console::pal::Black, c->Help());
                return 0;
            } else {
                term->PrintString(console::pal::Red,
                                  console::pal::Black,
                                  std::format("ERROR: Unknown command \"{}\"\n", arg));
            }
        }

        term->PrintString(console::pal::LightGray, console::pal::Black, R"(List of Commands
================

echo [msg [msg...]]:
  Prints messages to the console.

help [command]
  Either prints this list of commands or help for a specific command

exit
  Exits the dracon console.
)");
        return 0;
    }

    auto HelpCommand::Help() const -> const std::string & {
        static const std::string help_str =
          R"(help
  Prints list of commands.

help <command>
  Prints the help for a command.
)";
        return help_str;
    }

    ExitCommand::ExitCommand(const std::span<Argument> args) : Command{args} { }

    ExitCommand::~ExitCommand() { }

    auto ExitCommand::Run(Console *console) -> i32 {
        console->Exit();
        return 0;
    }

    auto ExitCommand::Help() const -> const std::string & {
        static const std::string help_str = R"(exit
  Exits the dracon console back to the host.
)";
        return help_str;
    }

} // namespace dracon
