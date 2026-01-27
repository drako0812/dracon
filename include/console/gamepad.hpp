#pragma once

#include <bitset>
#include "types.hpp"

namespace dracon::console {

    enum class GPButton : u8 {
        A,
        B,
        X,
        Y,
        UP,
        DOWN,
        LEFT,
        RIGHT,
        START,
    };

    struct GamePad {
        std::bitset<9> CurrentState;
        std::bitset<9> PreviousState;

        GamePad();
        auto IsButtonPressed(GPButton button) -> bool;
        auto IsButtonPressedP(GPButton button) -> bool;
        auto Update() -> void;
    };

} // namespace dracon::console
