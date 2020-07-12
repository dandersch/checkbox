#pragma once
#include "command.h"
#include "player.h"
#include "tile.h"

// functor for player movement
struct PlayerMover
{
    PlayerMover(f32 vx, f32 vy, b32 rightDir = false)
      : velocity(vx, vy)
      , rightDir(rightDir)
    {
    }
    void operator()(Player& p, f32) const
    {
        // movement not controllable mid-jump holding a box
        if ((p.m_state == Player::JUMPING || p.m_state == Player::FALLING) &&
            p.holding)
        {
            // direction change allowed when standing still
            if (p.velocity.x == 0.f) p.facingRight = rightDir;
            return;
        }

        if (!p.holding)
        {
            p.velocity += (1.5f * velocity);
            if (p.canJump) p.m_state = Player::RUNNING;
        }
        else
        {
            p.velocity += velocity;
            if (p.canJump) p.m_state = Player::WALKING;
        }

        p.facingRight = rightDir;
    }

    sf::Vector2f velocity;
    b32 rightDir;
};

static const Command moveLeftCmd = {
    derivedAction<Player>(PlayerMover(-75.f, 0.f, false)), ENTITY_PLAYER
};
static const Command moveRightCmd = {
    derivedAction<Player>(PlayerMover(75.f, 0.f, true)), ENTITY_PLAYER
};

static const Command jumpCmd = {
    derivedAction<Player>([](Player& p, f32) {
        if (p.canJump && p.holding)
        {
            p.fixedJump = true;
            p.velocity.y = -sqrtf(2.0f * 981.f * 100.f);
            p.m_state = Player::JUMPING;
            p.canJump = false;
        }
        else if (p.canJump)
        {
            p.velocity.y = -sqrtf(2.0f * 981.f * 120.f);
            p.m_state = Player::JUMPING;
            p.canJump = false;
        }
    }),
    ENTITY_PLAYER
};

static const Command respawnCmd = {
    derivedAction<Player>([](Player& p, f32) {
        if (p.checkpoint_box == p.holding && !p.dead)
            return; // holding the checkpoint
        if (p.checkpoint_box)
        {
            // TODO(dan): center/move view
            p.body->SetTransform(b2Vec2(pixelsToMeters(p.checkpoint_box->getPosition().x),
                                        pixelsToMeters(p.checkpoint_box->getPosition().y -
                                                       64.f)),
                                 0);
            // TODO(dan): play effect/animation
        }
        else
        {
            p.body->SetTransform(b2Vec2(pixelsToMeters(p.spawn_loc.x),
                                        pixelsToMeters(p.spawn_loc.y)),
                                 0);
        }

        p.dead = false;
    }),
    ENTITY_PLAYER
};

static const Command holdCmd = {
    derivedAction<Player>([](Player& p, f32) {
        if (p.holding && !p.dead)
        {
            auto forwardForce = p.forwardRay() * 15.f;
            p.holding->body->SetLinearVelocity(b2Vec2(forwardForce.x, -5.f));

            ((Tile*) p.holding)->m_sprite.setColor(sf::Color::White);
            p.holding = nullptr;
            // TODO apply force
        }
        else
        {
            p.holding = *p.holdables.begin();
        }
    }),
    ENTITY_PLAYER
};

static const Command dieCmd = { derivedAction<Player>([](Player& p, f32) {
                                    p.m_state = Player::DEAD;
                                }),
                                ENTITY_PLAYER };
