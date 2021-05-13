#pragma once
#include "pch.h"

class Entity;

enum CmdType
{
    CMD_NULL,
    CMD_MOVE_LEFT,
    CMD_MOVE_RIGHT,
    CMD_JUMP,
    CMD_RESPAWN,
    CMD_HOLD,
    CMD_RETRY,
    CMD_WIN
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
