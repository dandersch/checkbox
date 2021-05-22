#pragma once
#include "pch.h"

enum EntityType
{
    ENTITY_NONE         = 0,
    ENTITY_PLAYER       = 1 <<  0,
    ENTITY_ENEMY        = 1 <<  1,
    ENTITY_TILE         = 1 <<  2,
    ENTITY_CHECKPOINT   = 1 <<  3,
    ENTITY_HOLDABLE     = 1 <<  4,
    ENTITY_SPIKE        = 1 <<  5,
    ENTITY_SPIKE_BLOODY = 1 <<  6,
    ENTITY_COIN         = 1 <<  7,
    ENTITY_GOAL         = 1 <<  8
};

struct Command;

// TODO(dan): completely get rid of scenegraph, instead use array of entities &
// draw tiles first then rest
class Entity
  : public sf::Transformable
  , public sf::Drawable
  , private sf::NonCopyable
{
public:
    Entity();

    void attachChild(std::unique_ptr<Entity> child);
    std::unique_ptr<Entity> detachChild(const Entity& node);
    void update(f32 dt);
    sf::Transform getWorldTransform() const;
    void onCommand(const Command& command, f32 dt);

    virtual u32 getType() const;

    virtual sf::FloatRect getBoundingRect() const;

private:
    virtual void draw(sf::RenderTarget& target,
                      sf::RenderStates states) const override final;

    virtual void drawCurrent(sf::RenderTarget& target,
                             sf::RenderStates states) const
    {}

    virtual void updateCurrent(f32 dt) {}

    void updateChildren(f32 dt);

public:
    b2Body* body = nullptr;
    // EntityFlags flags;

    // scene graph
    std::vector<std::unique_ptr<Entity>> m_children;
    Entity* m_parent = nullptr;
};
