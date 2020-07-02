#pragma once
#include "pch.h"
#include "entity.h"
#include "command.h"
#include "player.h"
#include "tile.h"

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

        // swap entities if needed to make order consistent to order in EntityType
        if (body1->getType() > body2->getType())
        {
            e1 = body2;
            e2 = body1;
        }

        // early out
        if (!(e1->getType() == ENTITY_PLAYER)) return;

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

        if (e2->getType() & ENTITY_ENEMY)
        {
            // TODO(dan): make player die or sth similar
            player->dead = true;
        }

        if (e2->getType() & ENTITY_CHECKPOINT)
        {
            // TODO: play animation?
            // paint new checkpoint green and take away green from old one
            if (player->checkpoint_box)
                ((Tile*) player->checkpoint_box)->m_sprite.setColor(sf::Color::White);
            player->checkpoint_box = e2;
            ((Tile*) e2)->m_sprite.setColor(sf::Color::Green);
            return;
        }

        if (e2->getType() & ENTITY_TILE)
        {
            if (normal.x > 0) // touching right side of tile
            {
            }

            if (normal.x < 0) // touching left side of tile
            {
            }

            if (normal.y > 0) // touching bottom of tile
            {
                player->velocity.y = 0;
            }

            if (normal.y < 0) // touching top of tile
            {
                player->canJump = true;
                player->velocity.y = 0;
            }
        }
    }

    /*
    void EndContact(b2Contact* contact)
    {
    }
    */
};
