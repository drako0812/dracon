#include "console/keyboard.hpp"
#include "raylib.h"

namespace dracon::console {

    auto Keyboard::GetKey(i32 key) -> bool { return IsKeyDown(key); }

    auto Keyboard::GetKeyP(i32 key) -> bool {
        bool down = false;

        if ((key > 0) && (key < MAX_KEYBOARD_KEYS)) {
            Input * input = GetInput();
            if (input->Keyboard.previousKeyState[key] == 1) down = true;
        }

        return down;
    }

} // namespace dracon::console
