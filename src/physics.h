#pragma once

#include "pch.h"
#include "scenenode.h"
#include "command.h"
#include "player.h"

// conversion functions
constexpr double PIXELS_PER_METER = 32.0;
constexpr double PI = 3.14159265358979323846;

template<typename T>
constexpr T pixelsToMeters(const T& x)
{
    return x / PIXELS_PER_METER;
};

template<typename T>
constexpr T metersToPixels(const T& x)
{
    return x * PIXELS_PER_METER;
};

template<typename T>
constexpr T degToRad(const T& x)
{
    return PI * x / 180.f;
}

template<typename T>
constexpr T radToDeg(const T& x)
{
    return 180.f * x / PI;
};

class PlayerContactListener : public b2ContactListener
{
    void BeginContact(b2Contact* contact) override
    {
        SceneNode* body1 = (SceneNode*) contact->GetFixtureA()->GetBody()->GetUserData();
        SceneNode* body2 = (SceneNode*) contact->GetFixtureB()->GetBody()->GetUserData();

        // TODO this can be done better
        if (body1->getCategory() == Category::Player &&
            body2->getCategory() == Category::Scene) {
            //...world manifold is helpful for getting locations
            b2WorldManifold worldManifold;
            contact->GetWorldManifold(&worldManifold);

            float normalLength = 0.1f;
            b2Vec2 normalStart = worldManifold.points[0] -
                                 normalLength * worldManifold.normal;
            b2Vec2 normalEnd = worldManifold.points[0] +
                               normalLength * worldManifold.normal;

            b2Vec2 normal = normalStart - normalEnd;
            std::cout << normal.x << std::endl;
            std::cout << normal.y << std::endl;

            Player* player = static_cast<Player*>(body1);

            // coming from left
            if (normal.x > 0)
            {
            }

            // coming from right
            if (normal.x < 0)
            {
            }

            // touching ground
            if (normal.y > 0) {
                player->canJump = true;
                player->velocity.y = 0;
            }

            // touching bottom
            if (normal.y < 0) {
            }
        }

        if (body2->getCategory() == Category::Player &&
            body1->getCategory() == Category::Scene) {
            //...world manifold is helpful for getting locations
            b2WorldManifold worldManifold;
            contact->GetWorldManifold(&worldManifold);

            float normalLength = 0.1f;
            b2Vec2 normalStart = worldManifold.points[0] -
                                 normalLength * worldManifold.normal;
            b2Vec2 normalEnd = worldManifold.points[0] +
                               normalLength * worldManifold.normal;

            b2Vec2 normal = normalStart - normalEnd;
            std::cout << normal.x << std::endl;
            std::cout << normal.y << std::endl;

            Player* player = static_cast<Player*>(body2);

            // coming from left
            if (normal.x > 0)
            {
            }

            // coming from right
            if (normal.x < 0)
            {
            }

            // touching ground
            if (normal.y > 0) {
                player->canJump = true;
                player->velocity.y = 0;
            }

            // touching bottom
            if (normal.y < 0) {
            }
        }
    }

    /*
    void EndContact(b2Contact* contact)
    {
    }
    */
};
