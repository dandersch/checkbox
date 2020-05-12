#pragma once

class Animation
{
public:
    Animation(sf::Texture& texture, uint xcount, uint ycount, float length);
    ~Animation();

    sf::IntRect update(float deltatime);

private:
    float transition; // time after frame changes
    float totaltime;
    int spritewidth;
    int spriteheight;

    int count;
};
