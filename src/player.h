#pragma once

#include "pch.h"

#include "animation.h"

class Player
{
public:
    Player(sf::Texture* texture, float speed);
    ~Player();

    void update(float dtime);
    void draw(sf::RenderWindow& window);

private:
    sf::Sprite m_body;
    sf::Texture* m_tex;
    std::vector<Animation> m_anims;
    float m_speed = 1.0f;
};
