#include <iostream>
#include <memory>
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
#include <winres.h>
#include "../resource.h"
#include "console.hpp"
#include "console/const.hpp"
#include "player.hpp"
#include "raylib.h"

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

        result = console->PreRun("./ex108_time.lua");
        if (result.has_value()) {
            std::cerr << result.value() << "\n";
            return 1;
        }

        result = console->Run(std::make_shared<dracon::Player>(console->GetScript()));
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

    rlCloseWindow();
    return 0;
}
