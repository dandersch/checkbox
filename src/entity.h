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

class Enemy : public Entity
{
public:
    enum Type
    {
        Slave,
        Skeleton
    };

public:
    explicit Enemy(Type type, ResourcePool<sf::Texture>& textures);
    virtual void drawCurrent(sf::RenderTarget& target,
                             sf::RenderStates states) const override;

private:
    Type m_type;
    sf::Sprite m_sprite;
};
