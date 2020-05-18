#pragma once

#include "pch.h"

struct Command;

struct collisionInfo
{
    sf::FloatRect movement;
    sf::Vector2f halfsize;
    sf::Vector2f position;
    bool touchingGround = false;
};

class SceneNode
  : public sf::Transformable
  , public sf::Drawable
  , private sf::NonCopyable
{
public:
    SceneNode();

    void attachChild(std::unique_ptr<SceneNode> child);
    std::unique_ptr<SceneNode> detachChild(const SceneNode& node);
    void update(float dt);
    sf::Transform getWorldTransform() const;
    void onCommand(const Command& command, float dt);

    void checkCollisions(sf::FloatRect& collider, collisionInfo& movement);
    virtual unsigned int getCategory() const;

    virtual sf::FloatRect getBoundingRect() const;

private:
    virtual void draw(sf::RenderTarget& target,
                      sf::RenderStates states) const override final;

    virtual void drawCurrent(sf::RenderTarget& target,
                             sf::RenderStates states) const
    {}

    virtual void updateCurrent(float dt) {}

    void updateChildren(float dt);

public:
    std::vector<std::unique_ptr<SceneNode>> m_children;
    SceneNode* m_parent;
};
