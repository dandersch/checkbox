#include "animation.hpp"

Animation::Animation(sf::Texture& texture, uint xcount, uint ycount,
                     float length)
{
    this->spritewidth = texture.getSize().x / xcount;
    this->spriteheight = texture.getSize().y / ycount;

    transition = (float)length / xcount;

    texture.setRepeated(true);
}

Animation::~Animation() {}

sf::IntRect Animation::update(float deltatime)
{
    totaltime += deltatime;

    if (totaltime >= transition) {
        totaltime -= transition;

        count++;
    }

    return sf::IntRect(count * spritewidth, 2 * spriteheight, spritewidth,
                       spriteheight);
}
