#include "levelgen.h"
#include "resourcepool.h"
#include "entity.h"
#include "world.h"

struct TileInfo
{
    sf::IntRect rect;
    Layer layer;
    // friction?
};

void buildLevel(std::map<u32, Tile*>& tilemap, b2World* world,
                ResourcePool<sf::Texture>& textures,
                ResourcePool<sf::Image>& levels, sf::Vector2u& maxMapSize,
                std::array<Entity*, LAYER_COUNT>& m_layerNodes,
                Player* player, std::string levelName)
{
    // lambda comparator to be able to create a map with sf::Color as key
    auto comparator = [](const sf::Color& c1, const sf::Color& c2) -> b32 {
        if (c1.r < c2.r) return true;
        else if (c1.r == c2.r && c1.g < c2.g)
            return true;
        else if (c1.r == c2.r && c1.g == c2.g && c1.b < c2.b)
            return true;
        return false;
    };

    std::map<sf::Color, TileInfo, decltype(comparator)> colorMap(comparator);

    auto& levelTex = textures.get("simple_tiles_32x32.png");
    auto& spikeTex = textures.get("basicdungeontileset.png");
    auto& level0 = levels.get("level1.png");

    maxMapSize = level0.getSize();

    // fill the colormap
    u32 xCount = levelTex.getSize().x / 32;
    u32 yCount = levelTex.getSize().y / 32;
    for (u32 y = 0; y < yCount; y++)
    {
        for (u32 x = 0; x < xCount; x++)
        {
            // decide on foreground/background
            if (y == 0)
                colorMap[level0.getPixel(x, y)] = {
                    sf::IntRect(x * 32, y * 32, 32, 32), LAYER_MID
                };
            else if (y == 1)
                colorMap[level0.getPixel(x, y)] = {
                    sf::IntRect(x * 32, y * 32, 32, 32), LAYER_BACK
                };

            // remove metadata
            level0.setPixel(x, y, sf::Color::White);
        }
    }

    u32 playercount = 0;
    for (u32 y = 0; y < level0.getSize().y; y++)
    {
        for (u32 x = 0; x < level0.getSize().x; x++)
        {
            sf::Color sample = level0.getPixel(x, y);

            // Player generation
            if (sample == sf::Color::Blue)
            {
                playercount++;
                player->setPosition(x * 32, y * 32);
                player->body = createBox(world, player->getPosition().x,
                                         player->getPosition().y,
                                         PLAYER_WIDTH / 2,
                                         PLAYER_HEIGHT - 2, // TODO(dan):
                                                            // workaround for
                                                            // "better"
                                                            // collision
                                         b2_dynamicBody, player, player);
                player->body->SetFixedRotation(true);
                player->spawn_loc = sf::Vector2i(x * 32, y * 32);
                continue;
            }

            // spike generation
            if (sample == sf::Color::Red)
            {
                // 0 5
                std::unique_ptr<Tile> spike(new Tile(spikeTex,
                                                     sf::IntRect(5 * 32, 0 * 32,
                                                                 32, 32)));
                spike->setPosition(x * 32, y * 32);
                u32 id = tileIDfromCoords(x * 32, y * 32, maxMapSize);
                tilemap[id] = spike.get();
                spike->typeflags |= ENTITY_ENEMY;
                spike->body = createBox(world, x * 32, y * 32, 32, 32,
                                        b2_staticBody, spike.get(), player);
                m_layerNodes[LAYER_MID]->attachChild(std::move(spike));
                continue;
            }

            // checkpoint generation
            if (sample == sf::Color::Green)
            {
                std::unique_ptr<Tile> check(new Tile(spikeTex,
                                                     sf::IntRect(5 * 32, 16, 32,
                                                                 64 - 16)));
                check->setPosition(x * 32, y * 32);
                u32 id = tileIDfromCoords(x * 32, y * 32, maxMapSize);
                tilemap[id] = check.get();
                check->typeflags |= ENTITY_CHECKPOINT;
                check->body = createBox(world, x * 32, y * 32, 32, 64,
                                        b2_staticBody, check.get(), player,
                                        false);
                m_layerNodes[LAYER_BACK]->attachChild(std::move(check));
                continue;
            }

            if (sample == sf::Color::White) continue; // whitespace

            TileInfo& tInfo = colorMap.at(sample);
            std::unique_ptr<Tile> tile(new Tile(levelTex, tInfo.rect));
            tile->setPosition(x * 32, y * 32);
            u32 id = tileIDfromCoords(x * 32, y * 32, maxMapSize);
            tilemap[id] = tile.get();
            tile->body = createBox(world, x * 32, y * 32, 32, 32, b2_staticBody,
                                   tile.get(), player);

            // make background tiles non-collidable
            if (tInfo.layer == LAYER_BACK) tile->body->SetActive(false);

            m_layerNodes[tInfo.layer]->attachChild(std::move(tile));
        }
    }

    assert(playercount == 1);
}

b2Body* createBox(b2World* world, i32 posX, i32 posY, i32 sizeX, i32 sizeY,
                  b2BodyType type, void* userData, Player* player,
                  b32 collidable)
{
    b2BodyDef bodyDef;
    bodyDef.position.Set(pixelsToMeters<double>(posX),
                         pixelsToMeters<double>(posY));
    bodyDef.type = type;

    b2PolygonShape b2shape;
    b2shape.SetAsBox(pixelsToMeters<double>(sizeX / 2.f),
                     pixelsToMeters<double>(sizeY / 2.f));

    b2FixtureDef fixtureDef;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.4f;
    fixtureDef.restitution = 0.5f;
    fixtureDef.shape = &b2shape;
    if (!collidable) fixtureDef.isSensor = true;

    b2Body* body = world->CreateBody(&bodyDef);
    auto fixt = body->CreateFixture(&fixtureDef);
    fixt->SetUserData((void*) 0); // TODO(dan): use enums later

    // add sensor fixtures for player
    if (userData == player)
    {
        b2PolygonShape sensorShape;
        sensorShape.SetAsBox(pixelsToMeters<double>(sizeX * 2.f),
                             pixelsToMeters<double>(sizeY / 2.f));
        b2FixtureDef sensorFixtDef;
        sensorFixtDef.shape = &sensorShape;
        sensorFixtDef.isSensor = true;
        auto sensorFixt = body->CreateFixture(&sensorFixtDef);
        sensorFixt->SetUserData((void*) 1);
    }

    body->SetUserData(userData);

    return body;
}

u32 tileIDfromCoords(const u32 x, const u32 y, const sf::Vector2u MaxMapSize)
{
    return ((y / 32) * MaxMapSize.x) + (x / 32);
}
