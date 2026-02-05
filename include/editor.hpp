#pragma once

#include "mode.hpp"

namespace dracon {

    class Editor : public Mode {
      public:
        Editor();
        Editor(const Editor & other);
        Editor(Editor && other) noexcept;
        auto operator=(const Editor & other) -> Editor &;
        auto operator=(Editor && other) noexcept -> Editor &;
        ~Editor() override;
        auto OnLoad(Console * console) -> void override;
        auto OnUnload(Console * console) -> void override;
        auto OnUpdate(Console * console) -> void override;
        auto OnPreRender(Console * console) -> void override;
        auto OnRender(Console * console) -> void override;
        auto GetModeName() const -> const std::string & override;

      private:
        bool m_show_editor_tab_bar;
        bool m_show_manager;
        bool m_show_code_editor;
    };

} // namespace dracon
