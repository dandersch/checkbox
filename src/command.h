#pragma once

#include "pch.h"

class Entity;

namespace Category {
enum Type
{
    None = 0,
    Scene = 1 << 0,
    Player = 1 << 1,
    Enemy = 1 << 2,
};
}

struct Command
{
    std::function<void(Entity&, f32)> action;
    u32 category;
};

// To avoid having to downcast from scenenode in action functions
template<typename GameObject, typename Function>
std::function<void(Entity&, f32)> derivedAction(Function fn)
{
    return [=](Entity& node, f32 dt) {
        // Check if cast is safe
        assert(dynamic_cast<GameObject*>(&node) != nullptr);

        // Downcast node and invoke function on it
        fn(static_cast<GameObject&>(node), dt);
    };
}
