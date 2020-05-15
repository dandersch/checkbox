#pragma once

#include "pch.h"

#include "entity.h"
#include "animation.h"

// forward declarations
template<typename Resource>
class ResourcePool;

class Player : public Entity
{
public:
    Player(ResourcePool<sf::Texture>& textures);

    virtual void updateCurrent(float dtime) override;
    //void draw(sf::RenderWindow& window);
    void createAnimations();

private:
    void restartAnimsExcept(int index);
    void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;

public:
    float speed = 1.0f;
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
