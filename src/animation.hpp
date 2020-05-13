#pragma once

struct Frame
{
    int x, y;
    float duration; // time after frame changes;
    bool flipped = false;
};

class Animation
{
public:
    Animation(sf::Texture& texture, uint xcount, uint ycount);
    ~Animation();

    sf::IntRect update(float deltatime);
    void add(int x, int y, float duration, bool flipped = false);

private:
    float m_time;
    uint m_framewidth;
    uint m_frameheight;
    std::vector<Frame> m_frames;
    uint m_index = 0;
};
