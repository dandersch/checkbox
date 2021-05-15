#pragma once
#include "command.h"
#include "player.h"
#include "entity.h"
#include "tile.h"
#include "event.h"

//void cmdFillUpHashMap();

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
            //return; // TODO(dan): fix
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

static const Command nullCmd = { [](Entity& e, f32) {}, ENTITY_NONE, CMD_NULL };

static const Command moveLeftCmd = {
    derivedAction<Player>(PlayerMover(-150.f, 0.f, false)), ENTITY_PLAYER,
    CMD_MOVE_LEFT
};
static const Command moveRightCmd = {
    derivedAction<Player>(PlayerMover( 150.f, 0.f, true)), ENTITY_PLAYER,
    CMD_MOVE_RIGHT
};

static const Command jumpCmd = {
    derivedAction<Player>([](Player& p, f32) {
        if (p.canJump && p.holding)
        {
            p.fixedJump = true;
            p.velocity.y = -sqrtf(2.0f * 981.f * 200.f);
            p.m_state = Player::JUMPING;
            p.canJump = false;
        }
        else if (p.canJump)
        {
            p.velocity.y = -sqrtf(2.0f * 981.f * 300.f);
            p.m_state = Player::JUMPING;
            p.canJump = false;
        }
    }),
    ENTITY_PLAYER, CMD_JUMP
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

            ((Checkbox*) p.checkpoint_box)->anim.done = false;
            // TODO(dan): play effect/animation
        }
        else
        {
            p.body->SetTransform(b2Vec2(pixelsToMeters(p.spawn_loc.x),
                                        pixelsToMeters(p.spawn_loc.y)),
                                 0);
        }

        Event evn(EventType::EVENT_PLAYER_RESPAWN);
        evn.args["dying"]    = (void*) !(p.m_state == Player::DEAD);
        //evn.args["position"] = (void*) &a;
        evn.args["target"]   = static_cast<void*>(&p);
        EventSystem::sendEvent(evn);

        p.dead = false;
    }),
    ENTITY_PLAYER, CMD_RESPAWN
};

static const Command holdCmd = {
    derivedAction<Player>([](Player& p, f32) {
        if (p.holding && !p.dead)
        {
            auto forwardForce = p.forwardRay() * 15.f;
            p.holding->body->SetLinearVelocity(b2Vec2(forwardForce.x, -12.f));

            ((Tile*) p.holding)->m_sprite.setColor(sf::Color::White);
            p.holding = nullptr;
            // TODO apply force
        }
        else
        {
            p.holding = *p.holdables.begin();
        }
    }),
    ENTITY_PLAYER, CMD_HOLD
};

static const Command retryCmd = { derivedAction<Player>([](Player& p, f32) {
                                      p.retry();
                                  }),
                                  ENTITY_PLAYER, CMD_RETRY };

static const Command winCmd = { derivedAction<Player>([](Player& p, f32) {
                                    p.m_state = Player::CELEBRATING;
                                }),
                                ENTITY_PLAYER, CMD_WIN };

/*
static const Command dieCmd = { derivedAction<Player>([](Player& p, f32) {
                                    p.m_state = Player::DEAD;
                                }),
                                ENTITY_PLAYER };
*/

// TODO(dan): doesn't work
/*
static std::map<u32, Command> cmdFromType({
        { CMD_NULL,       nullCmd },
        { CMD_MOVE_LEFT,  moveLeftCmd },
        { CMD_MOVE_RIGHT, moveRightCmd },
        { CMD_JUMP,       jumpCmd },
        { CMD_RESPAWN,    respawnCmd },
        { CMD_HOLD,       holdCmd },
    });
*/
