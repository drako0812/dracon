#include <format>
#include <iostream>
#include "console.hpp"
#include "console/const.hpp"
#include "raylib.h"
#include "types.hpp"
#include "util.hpp"

int main() {
    InitWindow(dracon::console::FRAMEBUFFER_PIX_WIDTH * 2, dracon::console::FRAMEBUFFER_PIX_HEIGHT * 2, "dracon");

    {
        auto console = std::make_unique<dracon::Console>();
        dracon::Console::SetInstance(console.get());

        auto result = console->Init();
        if (result.has_value()) {
            std::cerr << result.value() << "\n";
            return 1;
        }

        result = console->Run("./ex108_time.lua");
        if (result.has_value()) {
            std::cerr << result.value() << "\n";
            return 1;
        }

        result = console->Deinit();
        if (result.has_value()) {
            std::cerr << result.value() << "\n";
            return 1;
        }
    }

    CloseWindow();
    return 0;
}
