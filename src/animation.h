#pragma once

#include "pch.h"

struct Frame
{
    i32 x, y;
    f32 duration; // time after frame changes;
    // individual flipped frames not supported right now
    // bool flipped = false; // frame is flipped vertically
};

class Animation
{
public:
    Animation(sf::Vector2u textureSize, u32 xcount, u32 ycount);

    sf::IntRect update(f32 dtime);
    void add(i32 x, i32 y, f32 duration, b32 flipped = false);

    inline void restart() { m_index = 0; };

public:
    b32 looped = true;
    b32 flipped = false; // whole animation is flipped vertically

private:
    f32 m_time = 0;
    u32 m_framewidth;
    u32 m_frameheight;
    std::vector<Frame> m_frames;
    u32 m_index = 0;
};
