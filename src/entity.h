#pragma once

#include "pch.h"
#include "scenenode.h"

// forward declarations
template<typename Resource>
class ResourcePool;

class Entity : public SceneNode
{
private:
    virtual void updateCurrent(float dt) override { move(velocity * dt); }

public:
    sf::Vector2f velocity = sf::Vector2f(0, 0);
};
