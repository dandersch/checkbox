#pragma once

#include "pch.h"

// TODO large enough?
// 500 * 4B = 2000 Bytes
#define MAX_EVENT_SUBS 500

enum class EventType
{
    EVENT_PLAYER_COIN_PICKUP,
    EVENT_PLAYER_DIED
};

struct Event
{
    EventType type;
    std::unordered_map<std::string, void*> args;

    Event(EventType type) : type(type)
    {
        args = std::unordered_map<std::string, void*>();
    };
};

class EventSystem
{
public:
    static void subscribe(std::function<void(const Event&)> handler)
    {
        onEvent[sub_count++] = handler;
    }

    static void sendEvent(const Event& evn)
    {
        if (sub_count == 0) return;
        for (u32 i = 0; i < sub_count; i++) onEvent[i](evn);
    }

    // TODO unsubscribe method

private:
    static std::function<void(const Event&)> onEvent[MAX_EVENT_SUBS];
    static u32 sub_count;
};
