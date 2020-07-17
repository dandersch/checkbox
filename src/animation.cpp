#include "pch.h"
#include "animation.h"

Animation::Animation(sf::Vector2u textureSize, u32 xcount, u32 ycount)
{
    m_framewidth = textureSize.x / xcount;
    m_frameheight = textureSize.y / ycount;
}

sf::IntRect Animation::update(f32 dtime)
{
    m_time += dtime;

    Frame& current = m_frames[m_index];

    if (m_time >= current.duration) {
        m_time -= current.duration;
        m_index++;
    }

    if (m_index >= m_frames.size()) {
        if (looped) m_index = 0;
        else m_index = m_frames.size() - 1;
        done = true;
    }

    if (flipped)
        return sf::IntRect((current.x + 1) * m_framewidth,
                           current.y * m_frameheight, -m_framewidth,
                           m_frameheight);
    else
        return sf::IntRect(current.x * m_framewidth, current.y * m_frameheight,
                           m_framewidth, m_frameheight);
}

void Animation::add(i32 x, i32 y, f32 duration, b32 flipped)
{
    m_frames.push_back({ x, y, duration });
}
