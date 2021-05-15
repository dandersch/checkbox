#pragma once
#include "pch.h"
#include "entity.h"
#include "animation.h"

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
        setPosition(metersToPixels(body->GetPosition().x),
                    metersToPixels(body->GetPosition().y));
    };

    inline u32 getType() const override { return typeflags; }

protected:
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
    u32 typeflags = ENTITY_TILE;
    b32 moving = false;
    mutable b32 shouldDraw = false;
    b32 exists = true;

//private:
    sf::Sprite m_sprite;
};

class Coin : public Tile
{
public:
    Coin(const sf::Texture& tile_sheet, const sf::IntRect& tile_rect,
         bool purple = false)
      : Tile(tile_sheet, tile_rect)
      , anim(tile_sheet.getSize(), 23, 14)
    {
        if (!purple) {
            anim.add(15, 0, 0.1f);
            anim.add(16, 0, 0.1f);
            anim.add(17, 0, 0.1f);
            anim.add(18, 0, 0.1f);
            value = 1;
        } else {
            anim.add(15, 1, 0.1f);
            anim.add(16, 1, 0.1f);
            anim.add(17, 1, 0.1f);
            anim.add(18, 1, 0.1f);
            value = 5;
        }
        anim.looped = true;
    }

    virtual void drawCurrent(sf::RenderTarget& target,
                             sf::RenderStates states) const override
    {
        if (!collected) Tile::drawCurrent(target, states);
    }

    virtual void updateCurrent(f32 dt) override {
        Tile::updateCurrent(dt);
        m_sprite.setTextureRect(anim.update(dt));
    }

    u32 value;
    Animation anim;
    b32 collected = false;
};

class Checkbox : public Tile
{
public:
    Checkbox(const sf::Texture& tile_sheet, const sf::IntRect& tile_rect)
      : Tile(tile_sheet, tile_rect)
      , anim(tile_sheet.getSize(), 23, 14)
    {
        anim.add(18, 5, 0.15f);
        anim.add(17, 5, 0.15f);
        anim.add(16, 5, 0.15f);
        anim.add(15, 5, 0.15f);
        anim.looped = true;
    }

    virtual void updateCurrent(f32 dt) override {
        Tile::updateCurrent(dt);

        if (!anim.done)
        {
            m_sprite.setTextureRect(anim.update(dt));
        }
        else
        {
            m_sprite.setTextureRect({ 15 * 64,
                                       3 * 64, 64, 64 });
        }
    }

    b32 playAnim = false;
    Animation anim;
};

class Corpse : public Tile
{
public:
    Corpse(const sf::Texture& tile_sheet, bool facingRight, bool dying = false)
      : Tile(tile_sheet, { 7 * 128, 2 * 128, 128, 128 })
    {
        if (facingRight)
            m_sprite.setTextureRect({ 7 * 128, 2 * 128, 128, 128 });
        else
            m_sprite.setTextureRect({ 8 * 128, 2 * 128, -128, 128 });

        if (dying)
        {
            anim = new Animation(tile_sheet.getSize(), 8, 9);
            anim->add(0, 2, 0.08f);
            anim->add(1, 2, 0.08f);
            anim->add(2, 2, 0.08f);
            anim->add(3, 2, 0.08f);
            anim->add(4, 2, 0.08f);
            anim->add(5, 2, 0.08f);
            anim->add(6, 2, 0.08f);
            anim->add(7, 2, 0.08f);
            anim->looped  = false;
            anim->flipped = !facingRight;
        }
    }

    ~Corpse() { free(anim); }

    virtual void updateCurrent(f32 dt) override {
        Tile::updateCurrent(dt);
        if (anim) m_sprite.setTextureRect(anim->update(dt));
    }

    virtual void drawCurrent(sf::RenderTarget& target,
                             sf::RenderStates states) const override
    {
        if (exists) target.draw(m_sprite, states);
    }

    Animation* anim = nullptr;
};
