#pragma once

#include "pch.h"
#include "scenenode.h"

class SpriteNode : public SceneNode
{
public:
    explicit SpriteNode(const sf::Texture& texture)
      : m_sprite(texture)
    {}

    SpriteNode(const sf::Texture& texture, const sf::IntRect& rect)
      : m_sprite(texture, rect)
    {}

private:
    virtual void drawCurrent(sf::RenderTarget& target,
                             sf::RenderStates states) const override
    {
        target.draw(m_sprite, states);
    }

private:
    sf::Sprite m_sprite;
};
