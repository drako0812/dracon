#include "player.hpp"
#include <chrono>
#include "console.hpp"
#include "prompt.hpp"
#include "sol/types.hpp"

namespace dracon {

    Player::Player(ptr<sol::protected_function> script) : Mode{}, m_running{false}, m_script{script}, m_MAIN_func{} { }
    Player::Player(const Player & other) :
      Mode{other},
      m_running{other.m_running},
      m_script{other.m_script},
      m_MAIN_func{other.m_MAIN_func} { }
    Player::Player(Player && other) noexcept :
      Mode{std::move(other)},
      m_running{other.m_running},
      m_script{other.m_script},
      m_MAIN_func{std::move(other.m_MAIN_func)} { }
    auto Player::operator=(const Player & other) -> Player & {
        Mode::operator=(other);
        m_running   = other.m_running;
        m_script    = other.m_script;
        m_MAIN_func = other.m_MAIN_func;
        return *this;
    }
    auto Player::operator=(Player && other) noexcept -> Player & {
        Mode::operator=(std::move(other));
        m_running   = other.m_running;
        m_script    = other.m_script;
        m_MAIN_func = std::move(other.m_MAIN_func);
        return *this;
    }
    Player::~Player() { m_MAIN_func = sol::nil; }
    auto Player::OnLoad(Console * console) -> void {
        if (!console->IsCartLoaded()) {
            console->SetNewMode(std::make_shared<Prompt>("Error: No Cartridge Loaded."));
            return;
        }

        auto & lua = console->GetLua();

        auto   load_status = m_script->valid();

        if (!load_status) {
            console->SetNewMode(std::make_shared<Prompt>("Error: Unable to load script"));
            return;
        }

        console->SetScriptStartTime(std::chrono::high_resolution_clock::now());

        auto script_return = m_script->call();

        auto script_status = script_return.status();
        if (script_status != sol::call_status::ok) {
            std::string msg;
            switch (script_status) {
                case sol::call_status::file: msg = "Lua File Error"; break;
                case sol::call_status::gc: msg = "Lua Garbage Collector Error"; break;
                case sol::call_status::handler: msg = "Lua Handler Error"; break;
                case sol::call_status::memory: msg = "Lua Memory Error"; break;
                case sol::call_status::runtime: msg = "Lua Runtime Error"; break;
                case sol::call_status::syntax: msg = "Lua Syntax Error"; break;
                case sol::call_status::yielded: msg = "Lua Yielded Error"; break;
                default: UNREACHABLE; break;
            }
            console->SetNewMode(std::make_shared<Prompt>(
              std::format("Error: Lua Error occurred while running script: {}\n{}", msg, sol::error(script_return).what())));
            return;
        }

        m_MAIN_func = lua["MAIN"];
        if (!m_MAIN_func.valid()) {
            console->SetNewMode(std::make_shared<Prompt>("Error: Unable to retrieve `MAIN` function from script"));
            return;
        }
    }
    auto Player::OnUnload(Console * console) -> void { m_MAIN_func = sol::nil; }
    auto Player::OnUpdate(Console * console) -> void {
        console->GetGamePad().Update();

        auto MAIN_result = m_MAIN_func.call();
        if (MAIN_result.status() != sol::call_status::ok) {
            std::string msg;
            switch (MAIN_result.status()) {
                case sol::call_status::file: msg = "Lua File Error"; break;
                case sol::call_status::gc: msg = "Lua Garbage Collector Error"; break;
                case sol::call_status::handler: msg = "Lua Handler Error"; break;
                case sol::call_status::memory: msg = "Lua Memory Error"; break;
                case sol::call_status::runtime: msg = "Lua Runtime Error"; break;
                case sol::call_status::syntax: msg = "Lua Syntax Error"; break;
                case sol::call_status::yielded: msg = "Lua Yielded Error"; break;
                default: UNREACHABLE; break;
            }
            console->SetNewMode(
              std::make_shared<Prompt>(std::format("Error: Lua Error occurred while running MAIN function: {}\n{}",
                                                   msg,
                                                   sol::error(MAIN_result).what())));
            return;
        }

        if (auto errs = console->CallOnFrameCallbacks(); !errs.empty()) {
            std::string msg;
            for (const auto & e : errs) { msg += e; }
            console->SetNewMode(
              std::make_shared<Prompt>(std::format("Error: Error(s) occurred while running OnFrame callbacks:\n{}", msg)));
        }
    }
    auto Player::OnPreRender(Console * console) -> void { (void)console; }
    auto Player::OnRender(Console * console) -> void { (void)console; }
    auto Player::GetModeName() const -> const std::string & {
        static const std::string name = "player";
        return name;
    }

} // namespace dracon
