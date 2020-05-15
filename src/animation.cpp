#include "pch.h"

#include "animation.h"

// TODO pass size directly (?)
Animation::Animation(sf::Vector2u textureSize, uint xcount, uint ycount)
{
    m_framewidth = textureSize.x / xcount;
    m_frameheight = textureSize.y / ycount;
}

Animation::~Animation() {}

sf::IntRect Animation::update(float dtime)
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
    }

    if (current.flipped)
        return sf::IntRect((current.x + 1) * m_framewidth,
                           current.y * m_frameheight, -m_framewidth,
                           m_frameheight);
    else
        return sf::IntRect(current.x * m_framewidth, current.y * m_frameheight,
                           m_framewidth, m_frameheight);
}

void Animation::add(int x, int y, float duration, bool flipped)
{
    m_frames.push_back({ x, y, duration, flipped });
}
