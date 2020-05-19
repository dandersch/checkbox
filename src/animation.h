#pragma once

#include "pch.h"

struct Frame
{
    int x, y;
    float duration; // time after frame changes;
    // individual flipped frames not supported right now
    // bool flipped = false; // frame is flipped vertically
};

class Animation
{
public:
    Animation(sf::Vector2u textureSize, uint xcount, uint ycount);

    sf::IntRect update(float dtime);
    void add(int x, int y, float duration, bool flipped = false);

    inline void restart() { m_index = 0; };

public:
    bool looped = true;
    bool flipped = false; // whole animation is flipped vertically

private:
    float m_time = 0;
    uint m_framewidth;
    uint m_frameheight;
    std::vector<Frame> m_frames;
    uint m_index = 0;
};
