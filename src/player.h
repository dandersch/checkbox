#pragma once

#include "pch.h"

#include "animation.h"

class Player
{
public:
    Player();
    ~Player();

    void update(float dtime);
    void draw(sf::RenderWindow& window);
    void setTexture(const sf::Texture& texture);

private:
    void restartAnimsExcept(int index);

public:
    float speed = 1.0f;
    sf::Vector2f movement = sf::Vector2f(0.f, 0.f);
    sf::Sprite body;

private:
    std::vector<Animation> m_anims;
    enum PlayerState
    {
        IDLE,
        WALKING_RIGHT,
        WALKING_LEFT,
        DEAD,
        RUN_RIGHT,
        RUN_LEFT
    } m_state;
};
