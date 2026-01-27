#pragma once

#include "types.hpp"

namespace dracon::console {

    struct Keyboard {
        auto GetKey(i32 key) -> bool;
        auto GetKeyP(i32 key) -> bool;
    };

} // namespace dracon::console
