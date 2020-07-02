#pragma once
#include "pch.h"
#include "entity.h"

extern bool g_cull_tiles;

// TODO(dan): dynamic bodys shouldn't be considered tiles, then the update
// function can be empty
// NOTE(dan): tiles shouldn't be entities (?)
class Tile : public Entity
{
public:
    Tile(const sf::Texture& tile_sheet, const sf::IntRect& tile_rect)
      : m_sprite(tile_sheet, tile_rect)
    {
        m_sprite.setOrigin(tile_rect.width / 2.0f, tile_rect.height / 2.0f);
    }

    virtual sf::FloatRect getBoundingRect() const override
    {
        return getWorldTransform().transformRect(m_sprite.getGlobalBounds());
    };

    virtual void updateCurrent(f32 dt) override
    {
        // TODO(dan): not needed for (static) tiles
        setRotation(radToDeg(body->GetAngle()));
        setPosition(metersToPixels(body->GetPosition().x), metersToPixels(body->GetPosition().y));
    };

    inline u32 getType() const override { return typeflags; }

private:
    virtual void drawCurrent(sf::RenderTarget& target,
                             sf::RenderStates states) const override
    {
        // can be turned on/off for debugging
        if (g_cull_tiles)
        {
            // tilemap culling
            if (shouldDraw)
            {
                target.draw(m_sprite, states);
                if (!moving) shouldDraw = false;
            }
        }
        else
        {
            target.draw(m_sprite, states);
        }
    }

public:
    b2Body* body = nullptr;
    u32 typeflags = ENTITY_TILE;
    b32 moving = false;
    mutable b32 shouldDraw = false;

//private:
    sf::Sprite m_sprite;
};
