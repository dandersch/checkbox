#pragma once
#include "pch.h"

class Entity;

// TODO(dan): move to entity
enum EntityType
{
    ENTITY_NONE       = 0,
    ENTITY_PLAYER     = 1 << 0,
    ENTITY_ENEMY      = 1 << 1,
    ENTITY_TILE       = 1 << 2
};

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
