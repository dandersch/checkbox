#pragma once
#include "pch.h"

class Entity;

// TODO(dan): move to entity
enum EntityType
{
    ENTITY_NONE         = 0,
    ENTITY_PLAYER       = 1 << 0,
    ENTITY_ENEMY        = 1 << 1,
    ENTITY_TILE         = 1 << 2,
    ENTITY_CHECKPOINT   = 1 << 3,
    ENTITY_HOLDABLE     = 1 << 4,
    ENTITY_SPIKE_UP     = 1 << 5,
    ENTITY_SPIKE_DOWN   = 1 << 6,
    ENTITY_SPIKE_LEFT   = 1 << 7,
    ENTITY_SPIKE_RIGHT  = 1 << 8,
    ENTITY_GOAL         = 1 << 9
};

enum CmdType
{
    CMD_NULL,
    CMD_MOVE_LEFT,
    CMD_MOVE_RIGHT,
    CMD_JUMP,
    CMD_RESPAWN,
    CMD_HOLD,
    CMD_RETRY
};

struct Command
{
    std::function<void(Entity&, f32)> action;
    u32 category;
    u32 cmdType = 0;
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
