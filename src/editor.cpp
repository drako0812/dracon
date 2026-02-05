#include "editor.hpp"
#include "editor/code_editor.hpp"
#include "editor/manager.hpp"
#include "imgui.h"
#include "imgui_internal.h"

namespace dracon {

    Editor::Editor() : Mode{}, m_show_editor_tab_bar{true}, m_show_manager{true}, m_show_code_editor{false} { }
    Editor::Editor(const Editor & other) :
      Mode{other},
      m_show_editor_tab_bar{other.m_show_editor_tab_bar},
      m_show_manager{other.m_show_manager},
      m_show_code_editor{other.m_show_code_editor} { }
    Editor::Editor(Editor && other) noexcept :
      Mode{std::move(other)},
      m_show_editor_tab_bar{other.m_show_editor_tab_bar},
      m_show_manager{other.m_show_manager},
      m_show_code_editor{other.m_show_code_editor} { }
    auto Editor::operator=(const Editor & other) -> Editor & {
        Mode::operator=(other);
        m_show_editor_tab_bar = other.m_show_editor_tab_bar;
        m_show_manager        = other.m_show_manager;
        m_show_code_editor    = other.m_show_code_editor;
        return *this;
    }
    auto Editor::operator=(Editor && other) noexcept -> Editor & {
        Mode::operator=(std::move(other));
        m_show_editor_tab_bar = other.m_show_editor_tab_bar;
        m_show_manager        = other.m_show_manager;
        m_show_code_editor    = other.m_show_code_editor;
        return *this;
    }
    Editor::~Editor() = default;
    auto Editor::OnLoad(Console * console) -> void { (void)console; }
    auto Editor::OnUnload(Console * console) -> void { (void)console; }
    auto Editor::OnUpdate(Console * console) -> void { (void)console; }
    auto Editor::OnPreRender(Console * console) -> void { (void)console; }
    auto Editor::OnRender(Console * console) -> void {
        if (ImGui::Begin("Editor")) {
            if (ImGui::BeginTabBar("editor_tab_bar")) {
                if (ImGui::BeginTabItem("Cart Man", &m_show_editor_tab_bar, ImGuiTabItemFlags_NoCloseButton)) {
                    m_show_code_editor = false;
                    m_show_manager     = true;
                    editor::CartManager(*console, m_show_manager);
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Code", &m_show_editor_tab_bar, ImGuiTabItemFlags_NoCloseButton)) {
                    m_show_manager     = false;
                    m_show_code_editor = true;
                    editor::CodeEditor(*console, m_show_code_editor);
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
    auto Editor::GetModeName() const -> const std::string & {
        static const std::string name = "editor";
        return name;
    }

} // namespace dracon
