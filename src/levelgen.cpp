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

// TODO
enum TileSheet
{
    TILE_SIZE = 32,
    SHEET_WIDTH = 288,
    SHEET_HEIGHT = 64
};

// lambda comparator to be able to create a map with sf::Color as key
auto comparator = [](const sf::Color& c1, const sf::Color& c2) -> b32 {
    if (c1.r < c2.r) return true;
    else if (c1.r == c2.r && c1.g < c2.g)
        return true;
    else if (c1.r == c2.r && c1.g == c2.g && c1.b < c2.b)
        return true;
    return false;
};

// internal
void fillColorMap(sf::Image& lvlImg, sf::Texture& levelTex,
                  std::map<sf::Color, TileInfo, decltype(comparator)>& colorMap);

void levelBuild(std::map<u32, Tile*>& tilemap, std::map<u32, Tile*>& tilemap_bg,
                b2World* world, ResourcePool<sf::Texture>& textures,
                ResourcePool<sf::Image>& levels, sf::Vector2u& maxMapSize,
                std::array<Entity*, LAYER_COUNT>& m_layerNodes, Player* player,
                const std::string& levelName_mid,
                const std::string& levelName_back, const std::string& tileSheet)
{
    std::map<sf::Color, TileInfo, decltype(comparator)> colorMap(comparator);
    auto& levelTex = textures.get(tileSheet);
    auto& lvlImg = levels.get(levelName_mid);
    auto& lvlImg_bg = levels.get(levelName_back);
    maxMapSize = lvlImg.getSize();
    fillColorMap(lvlImg, levelTex, colorMap);

    // MID GENERATION
    u32 playercount = 0;
    for (u32 y = 0; y < lvlImg.getSize().y; y++)
    {
        for (u32 x = 0; x < lvlImg.getSize().x; x++)
        {
            sf::Color sample = lvlImg.getPixel(x, y);

            // Player generation
            if (sample == sf::Color::Blue)
            {
                playercount++;
                player->setPosition(x * tile_width, y * tile_height);
                player->body = createBox(world, player->getPosition().x,
                                         player->getPosition().y,
                                         (PLAYER_WIDTH / 2) - 10,
                                         PLAYER_HEIGHT - 2, // TODO(dan):
                                                            // workaround for
                                                            // "better"
                                                            // collision
                                         b2_dynamicBody, player, player, true,
                                         0x0001, 0xFF0F);
                player->body->SetFixedRotation(true);
                player->spawn_loc = sf::Vector2i(x * tile_height,
                                                 y * tile_height);
                continue;
            }

            // spike generation
            if (sample == sf::Color::Red || sample == sf::Color(121, 0, 130) ||
                sample == sf::Color(130, 0, 65) ||
                sample == sf::Color(40, 0, 130))
            {
                TileInfo& tInfo = colorMap.at(sample);
                std::unique_ptr<Tile> spike(new Tile(levelTex, tInfo.rect));
                spike->setPosition(x * tile_width, y * tile_height);
                u32 id = levelTileIDfromCoords(x * tile_width, y * tile_height,
                                               maxMapSize);
                tilemap[id] = spike.get();
                spike->typeflags |= ENTITY_ENEMY;

                if (sample == sf::Color::Red)
                {
                    spike->typeflags |= ENTITY_SPIKE;
                    spike->body = createBox(world, x * tile_width,
                                            y * tile_height, tile_width,
                                            tile_height/2, b2_staticBody,
                                            spike.get(), player);
                    m_layerNodes[LAYER_MID]->attachChild(std::move(spike));
                }
                else if (sample == sf::Color(121, 0, 130))
                {
                    spike->typeflags |= ENTITY_SPIKE;
                    spike->body = createBox(world, x * tile_width,
                                            y * tile_height, tile_width/2,
                                            tile_height - 2, b2_staticBody,
                                            spike.get(), player);
                    m_layerNodes[LAYER_MID]->attachChild(std::move(spike));
                }
                else if (sample == sf::Color(130, 0, 65))
                {
                    spike->typeflags |= ENTITY_SPIKE;
                    spike->body = createBox(world, x * tile_width,
                                            y * tile_height, tile_width / 2,
                                            tile_height - 2, b2_staticBody,
                                            spike.get(), player);
                    m_layerNodes[LAYER_MID]->attachChild(std::move(spike));
                }
                else if (sample == sf::Color(40, 0, 130))
                {
                    spike->typeflags |= ENTITY_SPIKE;
                    spike->body = createBox(world, x * tile_width,
                                            y * tile_height, tile_width,
                                            tile_height / 2, b2_staticBody,
                                            spike.get(), player);
                    m_layerNodes[LAYER_MID]->attachChild(std::move(spike));
                }
                continue;
            }

            // checkpoint generation
            if (sample == sf::Color(0,255,1))
            {
                TileInfo& tInfo = colorMap.at(sample);
                std::unique_ptr<Tile> check(new Checkbox(levelTex, tInfo.rect));
                check->moving = true;
                check->shouldDraw = true;
                check->setPosition(x * tile_width, y * tile_height);
                check->typeflags = ENTITY_TILE | ENTITY_CHECKPOINT |
                                   ENTITY_HOLDABLE;
                check->body = createBox(world, x * tile_width, y * tile_height,
                                        tile_width, tile_height, b2_dynamicBody,
                                        check.get(), player, true, 0x0001, 0xFF0F);
                check->body->SetFixedRotation(true);
                player->checkboxes.push_back({ check.get(),
                                               sf::Vector2f(x * tile_width,
                                                            y * tile_height) });
                m_layerNodes[LAYER_MID]->attachChild(std::move(check));

                // struct checkbox { Entity* box; sf::Vector2f origPos;}
                // std::vector<checkbox> checkboxes;
                continue;
            }

            // goldcoin generation
            if (sample == sf::Color(238,156,1))
            {
                TileInfo& tInfo = colorMap.at(sample);
                std::unique_ptr<Tile> gold(new Coin(levelTex, tInfo.rect));
                gold->setPosition(x * tile_width, y * tile_height);
                u32 id = levelTileIDfromCoords(x * tile_width, y * tile_height,
                                               maxMapSize);
                ((Coin*) gold.get())->value = 1;
                tilemap[id] = gold.get();
                gold->typeflags = ENTITY_COIN;
                gold->body = createBox(world, x * tile_width, y * tile_height,
                                       tile_width, tile_height, b2_staticBody,
                                       gold.get(), player, false);
                gold->body->SetActive(true);
                m_layerNodes[LAYER_MID]->attachChild(std::move(gold));
                continue;
            }

            // goldcoin generation
            if (sample == sf::Color(61,53,179))
            {
                TileInfo& tInfo = colorMap.at(sample);
                std::unique_ptr<Tile> purp(new Coin(levelTex, tInfo.rect, true));
                purp->setPosition(x * tile_width, y * tile_height);
                u32 id = levelTileIDfromCoords(x * tile_width, y * tile_height,
                                               maxMapSize);
                tilemap[id] = purp.get();
                purp->typeflags = ENTITY_COIN;
                ((Coin*) purp.get())->value = 5;
                purp->body = createBox(world, x * tile_width, y * tile_height,
                                       tile_width, tile_height, b2_staticBody,
                                       purp.get(), player, false);
                purp->body->SetActive(true);
                m_layerNodes[LAYER_MID]->attachChild(std::move(purp));
                continue;
            }

            // goal generation, x = 15, y = 2
            if (sample == sf::Color(238,205,1))
            {
                TileInfo& tInfo = colorMap.at(sample);
                std::unique_ptr<Tile> goal(new Tile(levelTex, tInfo.rect));
                goal->setPosition(x * tile_width, y * tile_height);
                u32 id = levelTileIDfromCoords(x * tile_width, y * tile_height,
                                               maxMapSize);
                tilemap[id] = goal.get();
                goal->typeflags |= ENTITY_GOAL;
                goal->body = createBox(world, x * tile_width, y * tile_height,
                                       tile_width, tile_height, b2_staticBody,
                                       goal.get(), player, false);
                goal->body->SetActive(true);
                m_layerNodes[LAYER_MID]->attachChild(std::move(goal));
                continue;
            }

            if (sample == sf::Color::White) continue; // whitespace

            TileInfo& tInfo = colorMap.at(sample);
            std::unique_ptr<Tile> tile(new Tile(levelTex, tInfo.rect));
            tile->setPosition(x * tile_width, y * tile_height);
            u32 id = levelTileIDfromCoords(x * tile_width, y * tile_height,
                                           maxMapSize);
            tilemap[id] = tile.get();
            tile->body = createBox(world, x * tile_width, y * tile_height,
                                   tile_width, tile_height, b2_staticBody,
                                   tile.get(), player);

            m_layerNodes[LAYER_MID]->attachChild(std::move(tile));
        }
    }

    fillColorMap(lvlImg_bg, levelTex, colorMap);
    // BACKGROUND GENERATION
    for (u32 y = 0; y < lvlImg_bg.getSize().y; y++)
    {
        for (u32 x = 0; x < lvlImg_bg.getSize().x; x++)
        {
            sf::Color sample = lvlImg_bg.getPixel(x, y);
            if (sample == sf::Color::White) continue; // whitespace
            TileInfo& tInfo = colorMap.at(sample);
            std::unique_ptr<Tile> tile(new Tile(levelTex, tInfo.rect));
            tile->setPosition(x * tile_width, y * tile_height);
            u32 id = levelTileIDfromCoords(x * tile_width, y * tile_height,
                                           maxMapSize);
            tilemap_bg[id] = tile.get();
            tile->body = createBox(world, x * tile_width, y * tile_height,
                                   tile_width, tile_height, b2_staticBody,
                                   tile.get(), player);

            // make background tiles non-collidable
            tile->body->SetActive(false);

            m_layerNodes[LAYER_BACK]->attachChild(std::move(tile));
        }
    }

    assert(playercount == 1);
}

b2Body* createBox(b2World* world, i32 posX, i32 posY, i32 sizeX, i32 sizeY,
                  b2BodyType type, void* userData, Player* player,
                  b32 collidable, u16 categoryBits, u16 maskBits)
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
    fixtureDef.filter.categoryBits = categoryBits;
    fixtureDef.filter.maskBits = maskBits;
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

void levelPlaceBox(sf::Vector2f pos, b32 isStatic, b2World* world,
                   std::map<u32, Tile*>& tilemap,
                   std::array<Entity*, LAYER_COUNT>& m_layerNodes,
                   const sf::Vector2u maxMapSize, const sf::Texture& tile_sheet,
                   Player* player)
{
    u32 tilenr = 1;
    b2BodyType type = b2_dynamicBody;
    if (isStatic)
    {
        type = b2_staticBody;
        tilenr = 2;
    }

    // clamp boxes to tileraster
    auto xpos = std::round(pos.x / tile_width)  * tile_width;
    auto ypos = std::round(pos.y / tile_height) * tile_height;

    // there already is a collidable tile here so don't spawn another one
    auto tileIt = tilemap.find(levelTileIDfromCoords(xpos, ypos, maxMapSize));
    if (tileIt != tilemap.end())
    {
        if ((*tileIt).second->body->IsActive()) return;
    }

    std::unique_ptr<Tile> box(new Tile(tile_sheet,
                                       sf::IntRect(16 * tile_width,
                                                   3 * tile_height, tile_width,
                                                   tile_height)));
    box->moving = true;
    box->shouldDraw = true;
    box->setPosition(xpos, ypos);

    // testing dynamic body as checkpoint
    if (isStatic) box->typeflags = ENTITY_TILE;
    else
        box->typeflags = ENTITY_TILE | ENTITY_CHECKPOINT | ENTITY_HOLDABLE;

    box->body = createBox(world, xpos, ypos, tile_width, tile_height, type,
                          box.get(), player);

    box->body->SetFixedRotation(true);
    m_layerNodes[LAYER_MID]->attachChild(std::move(box));
}

u32 levelTileIDfromCoords(const u32 x, const u32 y, const sf::Vector2u MaxMapSize)
{
    return ((y / 64) * MaxMapSize.x) + (x / 64);
}

void fillColorMap(sf::Image& lvlImg, sf::Texture& levelTex,
                  std::map<sf::Color, TileInfo, decltype(comparator)>& colorMap)
{
    // fill the colormap
    u32 xCount = levelTex.getSize().x / tile_width;
    u32 yCount = levelTex.getSize().y / tile_height;
    for (u32 y = 0; y < yCount; y++)
    {
        for (u32 x = 0; x < xCount; x++)
        {
            // TODO(dan): decide on foreground/background
            colorMap[lvlImg.getPixel(x, y)] = {
                sf::IntRect(x * tile_width, y * tile_height, tile_width,
                            tile_height),
                LAYER_MID
            };

            // remove metadata
            lvlImg.setPixel(x, y, sf::Color::White);
        }
    }
}
