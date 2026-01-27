#include "console/gamepad.hpp"
#include "raylib.h"

namespace dracon::console {

    GamePad::GamePad() = default;
    auto GamePad::IsButtonPressed(GPButton button) -> bool { return CurrentState[static_cast<std::size_t>(button)]; }
    auto GamePad::IsButtonPressedP(GPButton button) -> bool { return PreviousState[static_cast<std::size_t>(button)]; }
    auto GamePad::Update() -> void {
        PreviousState = CurrentState;

        // TODO: Process Actual GamePad data

        CurrentState[0] = IsKeyDown(KEY_Z);
        CurrentState[1] = IsKeyDown(KEY_X);
        CurrentState[2] = IsKeyDown(KEY_A);
        CurrentState[3] = IsKeyDown(KEY_S);
        CurrentState[4] = IsKeyDown(KEY_UP);
        CurrentState[5] = IsKeyDown(KEY_DOWN);
        CurrentState[6] = IsKeyDown(KEY_LEFT);
        CurrentState[7] = IsKeyDown(KEY_RIGHT);
        CurrentState[8] = IsKeyDown(KEY_ENTER) || IsKeyDown(KEY_KP_ENTER);
    }

} // namespace dracon::console
