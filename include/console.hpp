#pragma once

#include <chrono>
#include <vector>
#include "cart.hpp"
#include "config.hpp"
#include "console/gamepad.hpp"
#include "console/gfx.hpp"
#include "console/keyboard.hpp"
#include "effolkronium/random.hpp"
#include "mode.hpp"
#include "raylib.h"

// SOL Configuration
#define SOL_ALL_SAFETIES_ON 1
#define SOL_PRINT_ERRORS    1

#include <optional>
#include <sol/state.hpp>
#include <string>
#include <string_view>
#include "sol/forward.hpp"
#include "types.hpp"

namespace dracon {

    class Console {
      public:
        Console();
        ~Console();
        auto        Init() -> std::optional<std::string>;
        auto        LoadNewCart() -> std::optional<std::string>;
        auto        GetCart() -> ptr<Cartridge>;
        auto        Deinit() -> std::optional<std::string>;
        auto        PreRun(std::optional<const std::string_view> script = std::nullopt) -> std::optional<std::string>;
        auto        Run(sptr<Mode> initial_mode) -> std::optional<std::string>;
        auto        Render() -> void;
        auto        IsRunning() const -> bool;
        auto        RegisterOnFrameCallback(sol::function func) -> std::optional<std::string>;
        auto        UnregisterOnFrameCallback(sol::function func) -> std::optional<std::string>;
        auto        PaletteDirty() -> void;
        auto        FrameBufferDirty() -> void;
        auto        GetGfx() -> console::Gfx &;
        auto        GetGamePad() -> console::GamePad &;
        static auto SetInstance(Console *con) -> void;
        static auto GetInstance() -> Console *;
        auto        GetTime() -> double;
        auto        GetTimeStamp() -> i32;
        auto        GetModeInstance() -> ptr<Mode>;
        auto        SetNewMode(sptr<Mode> mode) -> void;
        auto        SetScriptStartTime(std::chrono::high_resolution_clock::time_point tp) -> void;
        auto        IsCartLoaded() const -> bool;
        auto        GetLua() -> sol::state &;
        auto        CallOnFrameCallbacks() -> std::vector<std::string>;
        auto        GetScript() -> ptr<sol::protected_function>;
        auto        Exit() -> void;

      private:
        auto rebuildPal() -> void;
        auto rebuildFrameBuffer() -> void;
        auto loadLuaAPI() -> void;

      public:
        static Console *Instance;

      private:
        console::Gfx                                   m_gfx;
        console::GamePad                               m_gamepad;
        console::Keyboard                              m_keyboard;
        bool                                           m_running;
        bool                                           m_frmbuf_dirty;
        bool                                           m_pal_dirty;

        Image                                          m_frmbuf_copy_img;
        Image                                          m_pal_copy_img;
        Texture                                        m_frmbuf_copy;
        Texture                                        m_pal_copy;
        RenderTexture                                  m_rtex;
        Shader                                         m_pal_shdr;
        i32                                            m_pal_shdr_palettedTexture_loc;
        i32                                            m_pal_shdr_palette_loc;
        Texture                                        m_cursor_texture;
        i32                                            m_exit_value;
        sol::state                                     m_lua;
        effolkronium::random_local                     m_rand;
        std::vector<sol::protected_function>           m_on_frame_callbacks;
        std::chrono::high_resolution_clock::time_point m_script_start_time;

        Config                                         m_cfg;
        uptr<Cartridge>                                m_cart_slot;

        // sol::load_result                               m_script;
        sol::protected_function                        m_script;
        sptr<Mode>                                     m_mode_instance;
        sptr<Mode>                                     m_next_mode_instance;
    };

} // namespace dracon
