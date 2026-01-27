#include "console.hpp"
#include <chrono>
#include <format>
#include <iostream>
#include <string>
#include "console/const.hpp"
#include "console/gamepad.hpp"
#include "editor/code_editor.hpp"
#include "editor/manager.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "raylib.h"
#include "rlImGui.h"
#include "sol/types.hpp"
#include "util.hpp"

namespace dracon {

    static const std::string frag_shader_src =
      R"(#version 330

const int PALETTE_SIZE = 24;

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D palette;

out vec4 finalColor;

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord)*fragColor;

    finalColor = texture(palette, vec2((texelColor.r * (1.0 / (24.0 / 256.0))) + 0.0001, 0.5));
    //finalColor = texture(palette, vec2((texelColor.r / 24.0) + ((1.0 / 24.0) / 2.0), 0.5));
    //finalColor = texture(palette, texelColor.xy);
})";

    Console::Console() { }
    auto Console::Init() -> std::optional<std::string> {
        const auto tmp_frmbuf = GenImageColor(console::FRAMEBUFFER_PIX_WIDTH, console::FRAMEBUFFER_PIX_HEIGHT, BLACK);
        const auto tmp_pal    = GenImageColor(console::PALETTE_SIZE, 1, BLACK);
        const auto tmp_shdr   = LoadShaderFromMemory(nullptr, frag_shader_src.c_str());

        if (!IsShaderValid(tmp_shdr)) { return "Console::Init: Unable to load shader."; }
        m_rand                         = effolkronium::random_local{};
        m_gfx                          = {};
        m_gamepad                      = {};
        m_keyboard                     = {};
        m_running                      = false;
        m_frmbuf_dirty                 = true;
        m_pal_dirty                    = true;
        m_frmbuf_copy_img              = tmp_frmbuf;
        m_pal_copy_img                 = tmp_pal;
        m_frmbuf_copy                  = LoadTextureFromImage(tmp_frmbuf);
        m_pal_copy                     = LoadTextureFromImage(tmp_pal);
        m_rtex                         = LoadRenderTexture(console::FRAMEBUFFER_PIX_WIDTH, console::FRAMEBUFFER_PIX_HEIGHT);
        m_pal_shdr                     = tmp_shdr;
        m_pal_shdr_palettedTexture_loc = 0;
        m_pal_shdr_palette_loc         = GetShaderLocation(tmp_shdr, "palette");
        m_cursor_texture               = LoadTexture("cursor.png");
        m_exit_value                   = 0;
        m_lua                          = sol::state{};
        m_on_frame_callbacks           = {};
        auto        cfg_res            = Config::Load();
        bool        cfg_failed         = false;
        std::string cfg_str            = "";
        std::visit(overloads{[&](const std::string & s) {
                                 cfg_failed = true;
                                 cfg_str    = s;
                             },
                             [&](const Config & c) { m_cfg = c; }},
                   cfg_res);
        if (cfg_failed) { return cfg_str; }
        m_cart_slot.reset();

        return std::nullopt;
    }
    auto Console::LoadNewCart() -> std::optional<std::string> {
        m_cart_slot = std::make_unique<Cartridge>();
        return std::nullopt;
    }
    auto Console::GetCart() -> ptr<Cartridge> { return m_cart_slot.get(); }
    auto Console::Deinit() -> std::optional<std::string> {
        rlImGuiShutdown();

        std::string err_msg = "";
        if (auto res = m_cfg.Save(); res.has_value()) { err_msg = res.value(); }

        m_on_frame_callbacks.clear();
        m_lua.globals().clear();
        m_lua.stack_clear();
        m_lua.collect_garbage();
        m_lua = {};
        m_cart_slot.reset();
        UnloadTexture(m_cursor_texture);
        UnloadShader(m_pal_shdr);
        UnloadRenderTexture(m_rtex);
        UnloadTexture(m_pal_copy);
        UnloadTexture(m_frmbuf_copy);
        UnloadImage(m_pal_copy_img);
        UnloadImage(m_frmbuf_copy_img);

        ShowCursor();

        return err_msg.empty() ? std::optional<std::string>{std::nullopt} : std::optional<std::string>{err_msg};
    }
    auto Console::Run(std::optional<const std::string_view> script) -> std::optional<std::string> {
        rlImGuiSetup(true);

        // Load Lua default state.
        m_lua.open_libraries(sol::lib::base,
                             sol::lib::package,
                             sol::lib::table,
                             sol::lib::string,
                             sol::lib::math,
                             sol::lib::debug);

        loadLuaAPI();

        // Load the default font.
        m_gfx.FontData.LoadDefault();

        // Load default palette
        m_gfx.PalData.LoadDefault(this);

        // Load default sprites
        if (!m_gfx.SprData.LoadSpriteData(m_gfx.PalData, "data/demo-sprites.png")) {
            return "Unable to load sprite data from 'data/demo-sprites.png'\n";
        }

        HideCursor();

        if (auto ret = LoadNewCart(); ret.has_value()) { return ret; }

        m_running      = true;
        m_exit_value   = 0;
        m_pal_dirty    = true;
        m_frmbuf_dirty = true;

        if (script.has_value()) {
            auto load_result = m_lua.load_file(std::string(script.value()));
            auto load_status = load_result.status();
            if (load_status != sol::load_status::ok) {
                std::string msg;
                switch (load_status) {
                    case sol::load_status::file: msg = "Lua File Load Error"; break;
                    case sol::load_status::gc: msg = "Lua Garbage Collector Error"; break;
                    case sol::load_status::memory: msg = "Lua Memory Error"; break;
                    case sol::load_status::syntax: msg = "Lua Syntax Error"; break;
                    default: UNREACHABLE; break;
                }
                return std::format("Error loading script '{}': {}\n{}", script.value(), msg, sol::error(load_result).what());
            }

            m_script_start_time = std::chrono::high_resolution_clock::now();

            auto script_return = load_result.call();

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
                return std::format("Error running script '{}': {}\n{}",
                                   script.value(),
                                   msg,
                                   sol::error(script_return).what());
            }
        }

        sol::function MAIN_func = m_lua["MAIN"];
        if (!MAIN_func.valid()) { return std::format("Error retrieving MAIN function from script '{}'", script.value()); }

        while ((!WindowShouldClose()) && (m_running)) {
            m_gamepad.Update();

            auto MAIN_result = MAIN_func.call();
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
                return std::format("Error running MAIN function in '{}': {}\n{}",
                                   script.value(),
                                   msg,
                                   sol::error(MAIN_result).what());
            }

            for (const auto & func : m_on_frame_callbacks) {
                auto result = func.call();
                if (result.status() != sol::call_status::ok) {
                    std::string msg;
                    switch (result.status()) {
                        case sol::call_status::file: msg = "Lua File Error"; break;
                        case sol::call_status::gc: msg = "Lua Garbage Collector Error"; break;
                        case sol::call_status::handler: msg = "Lua Handler Error"; break;
                        case sol::call_status::memory: msg = "Lua Memory Error"; break;
                        case sol::call_status::runtime: msg = "Lua Runtime Error"; break;
                        case sol::call_status::syntax: msg = "Lua Syntax Error"; break;
                        case sol::call_status::yielded: msg = "Lua Yielded Error"; break;
                        default: UNREACHABLE; break;
                    }
                    return std::format("Error running on_frame_callback function: {}\n{}", msg, sol::error(result).what());
                }
            }

            Render();
        }

        return std::nullopt;
    }
    auto Console::Render() -> void {
        // static int count = 0;
        if (m_pal_dirty) { rebuildPal(); }

        if (m_frmbuf_dirty) { rebuildFrameBuffer(); }

        BeginTextureMode(m_rtex);
        BeginShaderMode(m_pal_shdr);
        // SetShaderValueTexture(m_pal_shdr, 0, m_frmbuf_copy);
        SetShaderValueTexture(m_pal_shdr, m_pal_shdr_palette_loc, m_pal_copy);
        // std::cerr << std::format("m_pal_shdr_palette_loc: {:x}\n", m_pal_shdr_palette_loc);
        DrawTextureRec(
          m_frmbuf_copy,
          Rectangle{.x = 0, .y = 0, .width = console::FRAMEBUFFER_PIX_WIDTH, .height = -console::FRAMEBUFFER_PIX_HEIGHT},
          Vector2{.x = 0, .y = 0},
          WHITE);
        // DrawRectangle(0, 0, console::FRAMEBUFFER_PIX_WIDTH, console::FRAMEBUFFER_PIX_HEIGHT, WHITE);
        EndShaderMode();
        EndTextureMode();

        // if (count <= 1) {
        //     if (count == 1) {
        //         auto img = LoadImageFromTexture(m_rtex.texture);
        //         ExportImage(img, "rtex_dump.png");
        //         UnloadImage(img);
        //     }
        //     count++;
        // }

        BeginDrawing();
        rlImGuiBegin();
        ClearBackground(BLACK);
        DrawTexturePro(
          m_rtex.texture,
          Rectangle{.x = 0, .y = 0, .width = console::FRAMEBUFFER_PIX_WIDTH, .height = console::FRAMEBUFFER_PIX_HEIGHT},
          Rectangle{.x      = 0,
                    .y      = 0,
                    .width  = static_cast<float>(GetRenderWidth()),
                    .height = static_cast<float>(GetRenderHeight())},
          Vector2{.x = 0, .y = 0},
          0,
          WHITE);
        if (m_cfg.ShowFPS) { DrawFPS(0, 0); }

        if (IsCursorOnScreen()) { DrawTexture(m_cursor_texture, GetMouseX(), GetMouseY(), WHITE); }

        static bool show_editor_tab_bar = true;
        static bool show_manager        = true;
        static bool show_code_editor    = false;
        if (ImGui::Begin("Editor")) {
            if (ImGui::BeginTabBar("editor_tab_bar")) {
                if (ImGui::BeginTabItem("Cart Man", &show_editor_tab_bar, ImGuiTabItemFlags_NoCloseButton)) {
                    show_code_editor = false;
                    show_manager     = true;
                    editor::CartManager(*this, show_manager);
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Code", &show_editor_tab_bar, ImGuiTabItemFlags_NoCloseButton)) {
                    show_manager     = false;
                    show_code_editor = true;
                    editor::CodeEditor(*this, show_code_editor);
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();

        rlImGuiEnd();
        EndDrawing();
    }
    auto Console::IsRunning() const -> bool { return m_running; }
    auto Console::RegisterOnFrameCallback(sol::function func) -> std::optional<std::string> {
        auto loc = std::find(std::begin(m_on_frame_callbacks), std::end(m_on_frame_callbacks), func);
        if (loc == std::end(m_on_frame_callbacks)) {
            m_on_frame_callbacks.push_back(func);
            return std::nullopt;
        }
        return "Console::RegisterOnFrameCallback: Warning, function already registered";
    }
    auto Console::UnregisterOnFrameCallback(sol::function func) -> std::optional<std::string> {
        auto loc = std::find(std::begin(m_on_frame_callbacks), std::end(m_on_frame_callbacks), func);
        if (loc != std::end(m_on_frame_callbacks)) {
            m_on_frame_callbacks.erase(loc);
            return std::nullopt;
        }
        return "Console::UnregisterOnFrameCallback: Warning, function not registered";
    }
    auto Console::PaletteDirty() -> void { m_pal_dirty = true; }
    auto Console::FrameBufferDirty() -> void { m_frmbuf_dirty = true; }
    auto Console::GetGfx() -> console::Gfx & { return m_gfx; }
    auto Console::SetInstance(Console * con) -> void { Instance = con; }
    auto Console::GetInstance() -> Console * { return Instance; }
    auto Console::GetTime() -> double {
        auto n    = std::chrono::high_resolution_clock::now();
        auto diff = n - m_script_start_time;
        return std::chrono::nanoseconds(diff).count() * 0.000001;
    }
    auto Console::GetTimeStamp() -> i32 {
        auto n = std::chrono::system_clock::now();
        return static_cast<i32>(std::chrono::duration_cast<std::chrono::seconds>(n.time_since_epoch()).count());
    }
    auto Console::rebuildPal() -> void {
        // static bool exported = false;
        for (i32 i = 0; i < console::PALETTE_SIZE; i++) {
            const auto pal_col = m_gfx.PalData.GetEntry(i);
            ImageDrawPixel(&m_pal_copy_img, i, 0, Color{.r = pal_col.R, .g = pal_col.G, .b = pal_col.B, .a = 255});
        }
        UpdateTexture(m_pal_copy, m_pal_copy_img.data);
        m_pal_dirty = false;

        // if (!exported) {
        //     ExportImage(m_pal_copy_img, "pal_dump.png");
        //     exported = true;
        // }
    }
    auto Console::rebuildFrameBuffer() -> void {
        // static int count = 0;

        for (i32 y = 0; y < console::FRAMEBUFFER_PIX_HEIGHT; y++) {
            for (i32 x = 0; x < console::FRAMEBUFFER_PIX_WIDTH; x++) {
                const auto pal_idx = m_gfx.FrameBuf.GetPixelFast(x, y);
                ImageDrawPixel(&m_frmbuf_copy_img,
                               x,
                               y,
                               Color{.r = static_cast<unsigned char>(pal_idx),
                                     .g = static_cast<unsigned char>(pal_idx),
                                     .b = static_cast<unsigned char>(pal_idx),
                                     .a = 255});
            }
        }
        UpdateTexture(m_frmbuf_copy, m_frmbuf_copy_img.data);
        m_frmbuf_dirty = false;

        // if (count <= 1) {
        //     if (count == 1) { ExportImage(m_frmbuf_copy_img, "frmbuf_dump.png"); }
        //     count++;
        // }
    }
    auto Console::loadLuaAPI() -> void {
        m_lua["print"] = [&](const std::string_view s) -> void { std::cout << std::format("[DRACON PRINT]{}", s); };
        m_lua["exit"]  = [&](i32 code) -> void {
            m_exit_value = code;
            m_running    = false;
        };
        m_lua["dofile"]   = [&](const std::string_view s) -> void { (void)s; };
        m_lua["loadfile"] = [&](const std::string_view s) -> void { (void)s; };
        m_lua["pix"]  = sol::overload([&](i32 x, i32 y, i32 color) -> void { m_gfx.FrameBuf.SetPixel(this, x, y, color); },
                                     [&](i32 x, i32 y) -> i32 { return m_gfx.FrameBuf.GetPixel(x, y).value_or(-1); });
        m_lua["cls"]  = [&](i32 color) -> void { m_gfx.FrameBuf.Cls(this, color); };
        m_lua["line"] = [&](i32 x1, i32 y1, i32 x2, i32 y2, i32 color) -> void {
            m_gfx.FrameBuf.Line(this, x1, y1, x2, y2, color);
        };
        m_lua["rect"] = [&](i32 x, i32 y, i32 w, i32 h, i32 color) -> void { m_gfx.FrameBuf.Rect(this, x, y, w, h, color); };
        m_lua["rectf"] = [&](i32 x, i32 y, i32 w, i32 h, i32 color) -> void {
            m_gfx.FrameBuf.RectF(this, x, y, w, h, color);
        };
        m_lua["circ"]  = [&](i32 x, i32 y, i32 r, i32 color) -> void { m_gfx.FrameBuf.Circ(this, x, y, r, color); };
        m_lua["circf"] = [&](i32 x, i32 y, i32 r, i32 color) -> void { m_gfx.FrameBuf.CircF(this, x, y, r, color); };
        m_lua["putch"] = [&](i32 x, i32 y, char ch, i32 color) -> void { m_gfx.FrameBuf.PutCh(this, x, y, ch, color); };
        m_lua["puts"]  = [&](i32 x, i32 y, const std::string_view s, i32 color) -> void {
            m_gfx.FrameBuf.PutS(this, x, y, s, color);
        };
        m_lua["register_on_frame"] = [&](sol::function func) -> void {
            auto ret = RegisterOnFrameCallback(func);
            if (ret.has_value()) { std::cerr << ret.value() << "\n"; }
        };
        m_lua["unregister_on_frame"] = [&](sol::function func) -> void {
            auto ret = UnregisterOnFrameCallback(func);
            if (ret.has_value()) { std::cerr << ret.value() << "\n"; }
        };
        m_lua["tri"] = [&](i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, i32 color) -> void {
            m_gfx.FrameBuf.Tri(this, x1, y1, x2, y2, x3, y3, color);
        };
        m_lua["trif"] = [&](i32 x1, i32 y1, i32 x2, i32 y2, i32 x3, i32 y3, i32 color) -> void {
            m_gfx.FrameBuf.TriF(this, x1, y1, x2, y2, x3, y3, color);
        };
        m_lua["spr"] = sol::overload(
          [&](i32 x, i32 y, i32 spr, i32 flip, i32 rot) -> void { m_gfx.FrameBuf.Spr(this, x, y, spr, flip, rot); },
          [&](i32 x, i32 y, i32 spr, i32 flip, i32 rot, i32 key) -> void {
              m_gfx.FrameBuf.Spr(this, x, y, spr, flip, rot, key);
          });
        m_lua["btn"] = [&](i32 button) -> bool { return m_gamepad.IsButtonPressed(static_cast<console::GPButton>(button)); };
        m_lua["btnp"] = [&](i32 button) -> bool {
            return m_gamepad.IsButtonPressedP(static_cast<console::GPButton>(button));
        };
        m_lua["key"]    = [&](i32 k) -> bool { return m_keyboard.GetKey(k); };
        m_lua["keyp"]   = [&](i32 k) -> bool { return m_keyboard.GetKeyP(k); };
        m_lua["time"]   = [&]() -> double { return GetTime(); };
        m_lua["tstamp"] = [&]() -> i32 { return GetTimeStamp(); };
    }

    Console * Console::Instance = nullptr;

} // namespace dracon
