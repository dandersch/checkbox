#pragma once
#include "pch.h"
#include "entity.h"
#include "command.h"
#include "player.h"
#include "tile.h"

#include "event.h"

extern bool create_joint;
extern b2Body* body_player;
extern b2Body* body_box;

// Collision Listener
class PlayerContactListener : public b2ContactListener
{
    // TODO(dan): only player-on-tile supported currently
    void BeginContact(b2Contact* contact) override
    {
        Entity* body1 = (Entity*) contact->GetFixtureA()->GetBody()->GetUserData();
        Entity* body2 = (Entity*) contact->GetFixtureB()->GetBody()->GetUserData();

        auto e1 = body1;
        auto e2 = body2;
        auto player_fixt = contact->GetFixtureA();
        auto other_fixt = contact->GetFixtureB();

        // swap entities if needed to make order consistent to order in EntityType
        if (body1->getType() > body2->getType())
        {
            e1 = body2;
            e2 = body1;
            player_fixt = contact->GetFixtureB();
            other_fixt = contact->GetFixtureA();
        }

        // early out
        if (!(e1->getType() == ENTITY_PLAYER)) return;

        // determine fixture
        i32 fixtureType = 0;
        if (player_fixt->GetUserData() == (void*) 1) fixtureType = 1;

        //...world manifold is helpful for getting locations
        b2WorldManifold worldManifold;
        contact->GetWorldManifold(&worldManifold);

        f32 normalLength = 0.1f;
        b2Vec2 normal1 = worldManifold.points[0] -
                         normalLength * worldManifold.normal;
        b2Vec2 normal2 = worldManifold.points[0] +
                         normalLength * worldManifold.normal;

        b2Vec2 normal;
        if (body1 == e1) normal = normal1 - normal2;
        else normal = normal2 - normal1;

        // TODO(dan): playerOnTile(e1,e2,normal)
        Player* player = static_cast<Player*>(e1);

        // player sensor fixture collision with holdable
        if (e2->getType() & ENTITY_HOLDABLE && fixtureType == 1)
        {
            player->holdables.insert(e2);
        }

        if (e2->getType() & ENTITY_GOAL && fixtureType == 0)
        {
            player->celebTimer.restart();
            player->gameWon = true;
        }

        if (e2->getType() & ENTITY_ENEMY && fixtureType == 0)
        {
            if (!player->dead) player->lifeCount--;
            player->holding = nullptr;
            player->dead = true;

            // TODO(dan): hardcoded
            // make spikes bloody
            sf::IntRect bloodyRect(0,0,64,64);
            if (e2->getType() & ENTITY_SPIKE_UP)
            {
                bloodyRect.left = 5 * 64;
                bloodyRect.top = 9 * 64;
            }
            else if (e2->getType() & ENTITY_SPIKE_DOWN)
            {
                bloodyRect.left = 5 * 64;
                bloodyRect.top = 11 * 64;
            }
            else if (e2->getType() & ENTITY_SPIKE_LEFT)
            {
                bloodyRect.left = 4 * 64;
                bloodyRect.top = 10 * 64;

            }
            else if (e2->getType() & ENTITY_SPIKE_RIGHT)
            {
                bloodyRect.left = 6 * 64;
                bloodyRect.top = 10 * 64;
            }

            ((Tile*) e2)->m_sprite.setTextureRect(bloodyRect);
        }

        if (e2->getType() & ENTITY_CHECKPOINT && fixtureType == 0)
        {
            player->checkpoint_box = e2;

            // TODO(dan): return early breaks things when checkpoint is a
            // collidable body
            // return;
        }

        if ((e2->getType() & ENTITY_COIN) && fixtureType == 0)
        {
            if (((Coin*) e2)->collected) return;
            Event evn(EventType::EVENT_PLAYER_COIN_PICKUP);
            evn.args["coin"] = (void*) e2;
            EventSystem::sendEvent(evn);
            return;
        }

        if (e2->getType() & ENTITY_TILE && fixtureType == 0)
        {
            // ignore collision with currently held entity
            if (player->holding == e2) return;

            if (normal.x > 0) // touching right side of tile
            {
                player->fixedJump = false;
            }

            if (normal.x < 0) // touching left side of tile
            {
                player->fixedJump = false;
            }

            if (normal.y > 0) // touching bottom of tile
            {
                player->velocity.y = 0;
            }

            if (normal.y < 0) // touching top of tile
            {
                player->fixedJump = false;
                player->canJump = true;
                player->velocity.y = 0;
            }
        }
    }

    // TODO(dan): use endcontact for removing holdables from set
    /*
    void EndContact(b2Contact* contact)
    {
    }
    */
};
