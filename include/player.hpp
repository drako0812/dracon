#pragma once

#include "mode.hpp"
#include "sol/sol.hpp"

namespace dracon {

    class Player : public Mode {
      public:
        Player(ptr<sol::protected_function> script);
        Player(const Player & other);
        Player(Player && other) noexcept;
        auto operator=(const Player & other) -> Player &;
        auto operator=(Player && other) noexcept -> Player &;
        ~Player() override;
        auto OnLoad(Console * console) -> void override;
        auto OnUnload(Console * console) -> void override;
        auto OnUpdate(Console * console) -> void override;
        auto OnPreRender(Console * console) -> void override;
        auto OnRender(Console * console) -> void override;
        auto GetModeName() const -> const std::string & override;

      private:
        bool                         m_running;
        ptr<sol::protected_function> m_script;
        sol::protected_function      m_MAIN_func;
    };

} // namespace dracon
