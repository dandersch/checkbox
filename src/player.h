#pragma once

#include "pch.h"

#include "animation.h"

class Player
{
public:
    Player(sf::Texture* texture);
    ~Player();

    void update(float dtime);
    void draw(sf::RenderWindow& window);

    // TODO
    // void setTexture(...)

private:
    void restartAnimsExcept(int index);

public:
    float speed = 1.0f;
    sf::Sprite m_body; // TODO private

private:
    sf::Texture* m_tex;
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
