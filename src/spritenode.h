#pragma once

#include "pch.h"
#include "scenenode.h"

class SpriteNode : public SceneNode
{
public:
    explicit SpriteNode(const sf::Texture& texture)
      : m_sprite(texture)
    {
        // TODO keep (?)
        m_sprite.setOrigin(texture.getSize().x / 2.0f,
                           texture.getSize().y / 2.0f);
    };

    SpriteNode(const sf::Texture& texture, const sf::IntRect& rect)
      : m_sprite(texture, rect)
    {
        // TODO keep (?)
        m_sprite.setOrigin(rect.width / 2.0f,
                           rect.height / 2.0f);
    }

    virtual sf::FloatRect getBoundingRect() const override
    {
        return getWorldTransform().transformRect(m_sprite.getGlobalBounds());
    };

    /*
    unsigned int getCategory() const override
    {
        return Category::Scene;
    }
    */

private:
    virtual void drawCurrent(sf::RenderTarget& target,
                             sf::RenderStates states) const override
    {
        target.draw(m_sprite, states);
    }

private:
    sf::Sprite m_sprite;
};
