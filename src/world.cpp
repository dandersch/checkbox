#include "world.h"
#include "entity.h"
#include "spritenode.h"
#include "player.h"
#include "command.h"

#define VIEW_HEIGHT 1280
#define VIEW_WIDTH  720

// TODO(dan): tilemap.cpp
static const char* tiletexfile = "simple_tiles_32x32.png";
u32 tileIDfromCoords(const u32 x, const u32 y);
std::map<u32, SpriteNode*> tilemap;
sf::Vector2u maxMapSize;
struct Tile
{
    sf::IntRect rect;
    Layer layer;
    // friction?
};
enum TileSheet
{
    TILE_SIZE = 32,
    SHEET_WIDTH = 288,
    SHEET_HEIGHT = 64
};

World::World(sf::RenderWindow& window)
  : m_window(window)
  , m_view(sf::Vector2f(640.f, 360.f), sf::Vector2f(VIEW_HEIGHT, VIEW_WIDTH))
  , m_textures(".png")
  , m_levels(".png")
  , m_player(nullptr)
  , playerTileContact() // collision response
  , world(b2Vec2(0.f, 25.f)) // TODO keep using higher gravity (?)
{
    loadTextures();
    buildScene();
    world.SetContactListener(&playerTileContact);
}

void World::update(f32 dt)
{
    { // VIEW UPDATE //////////////////////////////////////////////////////////
        // get player position on screen in pixels
        sf::Vector2u pPixelPos = (sf::Vector2u) m_window.mapCoordsToPixel(m_player->getPosition(),
                                                                          m_view);
        // get player position on screen in [0-1]
        sf::Vector2f pRelPos((f32) pPixelPos.x / m_window.getSize().x,
                             (f32) pPixelPos.y / m_window.getSize().y);

        // move view if player gets out of borders
        // TODO breaks if player near border and zooming in/resizing
        // TODO(dan): camera class/.cpp?
        if (pRelPos.x < 0.3f && m_player->velocity.x < 0.f)
        {
            m_view.move(m_player->velocity.x * dt, 0.f);
            m_view.setCenter(std::floor(m_view.getCenter().x),
                             m_view.getCenter().y);
        }
        if (pRelPos.x > 0.7f && m_player->velocity.x > 0.f)
        {
            m_view.move(m_player->velocity.x * dt, 0.f);
            m_view.setCenter(std::ceil(m_view.getCenter().x),
                             m_view.getCenter().y);
        }
        if (pRelPos.y < 0.3f && m_player->velocity.y < 0.f)
        {
            m_view.move(0.f, m_player->velocity.y * dt);
            m_view.setCenter(m_view.getCenter().x,
                             std::ceil(m_view.getCenter().y));
        }
        if (pRelPos.y > 0.7f && m_player->velocity.y > 0.f)
        {
            m_view.move(0.f, m_player->velocity.y * dt);
            m_view.setCenter(m_view.getCenter().x,
                             std::floor(m_view.getCenter().y));
        }

        // workaround to let player not get stuck outside of viewborders
        if (pRelPos.x < 0.28f) m_view.setCenter(m_player->getPosition());
        if (pRelPos.x > 0.72f) m_view.setCenter(m_player->getPosition());
        if (pRelPos.y < 0.28f) m_view.setCenter(m_player->getPosition());
        if (pRelPos.y > 0.72f) m_view.setCenter(m_player->getPosition());

        // m_view.setCenter(std::round(m_view.getCenter().x), std::round(m_view.getCenter().y));
    } //////////////////////////////////////////////////////////////////////////

    // reset velocity
    m_player->velocity.x = 0.f;

    while (!cmdQueue.empty()) {
        Command cmd = cmdQueue.front();
        cmdQueue.pop();
        m_scenegraph.onCommand(cmd, dt);
    }

    // Physics
    m_player->velocity.y += metersToPixels(world.GetGravity().y) * dt;
    m_player->body->SetLinearVelocity(b2Vec2(pixelsToMeters(m_player->velocity.x),
                                             pixelsToMeters(m_player->velocity.y)));
    world.Step(dt, (i32) 8, (i32) 3);

    m_scenegraph.update(dt);

    // workaround so y-velocity doesnt build up from gravity
    if (m_player->velocity.y > 2000.f) m_player->velocity.y = 2000.f;
}

void World::draw()
{
    m_window.setView(m_view);

    // TODO(dan): this maybe shouldn't happen in the drawloop, but putting it in
    // the update loop causes flickering...
    { // TILEMAP CULLING ////////////////////////////////////////////////
        u32 view_left   = std::round( (m_view.getCenter().x - (m_view.getSize().x / 2)) / 32) * 32;
        u32 view_top    = std::round( (m_view.getCenter().y - (m_view.getSize().y / 2)) / 32) * 32;
        u32 view_right  = std::round( (m_view.getCenter().x + (m_view.getSize().x / 2)) / 32) * 32;
        u32 view_bottom = std::round( (m_view.getCenter().y + (m_view.getSize().y / 2)) / 32) * 32;

        for (u32 y = view_top; y <= view_bottom; y += 32)
        {
            for (u32 x = view_left; x <= view_right; x += 32)
            {
                u32 id = tileIDfromCoords(x, y);
                if (tilemap.find(id) != tilemap.end())
                {
                    tilemap[id]->shouldDraw = true;
                }
            }
        }
    } ////////////////////////////////////////////////////////////////////

    m_window.draw(m_scenegraph);
}

// sort of a "preload"
void World::loadTextures()
{
}

void World::buildScene()
{
    // initialize nodes for every layer of the scene
    for (std::size_t i = 0; i < LAYER_COUNT; i++) {
        std::unique_ptr<Entity> layer(new Entity());
        m_layerNodes[i] = layer.get();
        m_scenegraph.attachChild(std::move(layer));
    }

    //////////////////////////////////////////////////
    // Generation of level from image:
    //////////////////////////////////////////////////

    // lambda comparator to be able to create a map with sf::Color as key
    auto comparator = [](const sf::Color& c1, const sf::Color& c2) -> b32 {
        if (c1.r < c2.r) return true;
        else if (c1.r == c2.r && c1.g < c2.g) return true;
        else if (c1.r == c2.r && c1.g == c2.g && c1.b < c2.b) return true;
        return false;
    };

    std::map<sf::Color, Tile, decltype(comparator)> colorMap(comparator);

    auto& levelTex = m_textures.get(tiletexfile);
    auto& level0 = m_levels.get("level1.png");

    maxMapSize = level0.getSize();

    // fill the colormap
    u32 xCount = levelTex.getSize().x / 32;
    u32 yCount = levelTex.getSize().y / 32;
    for (u32 y = 0; y < yCount; y++) {
        for (u32 x = 0; x < xCount; x++) {
            // decide on foreground/background
            if (y == 0)
                colorMap[level0.getPixel(x, y)] = {sf::IntRect(x * 32, y * 32, 32, 32), LAYER_MID};
            else if (y == 1)
                colorMap[level0.getPixel(x, y)] = {sf::IntRect(x * 32, y * 32, 32, 32), LAYER_BACK};

            // remove metadata
            level0.setPixel(x, y, sf::Color::White);
        }
    }

    u32 playercount = 0;
    for (u32 y = 0; y < level0.getSize().y; y++) {
        for (u32 x = 0; x < level0.getSize().x; x++) {
            sf::Color sample = level0.getPixel(x,y);

            // Player generation
            if (sample == sf::Color::Red) {
                playercount++;
                std::unique_ptr<Player> player(new Player(m_textures));
                m_player = player.get();
                m_view.setCenter(m_player->getPosition());
                m_layerNodes[LAYER_MID]->attachChild(std::move(player));
                m_player->setPosition(x * 32, y * 32);
                m_player->body = createBox(world, m_player->getPosition().x,
                                           m_player->getPosition().y,
                                           PLAYER_WIDTH / 2,
                                           PLAYER_HEIGHT - 2, // TODO(dan):
                                                              // workaround for
                                                              // "better"
                                                              // collision
                                           b2_dynamicBody, m_player);
                m_player->body->SetFixedRotation(true);
                continue;
            }

            if (sample == sf::Color::White) continue; // whitespace

            auto& layer_and_rect = colorMap[sample];
            std::unique_ptr<SpriteNode> tile(new SpriteNode(levelTex,
                                                            layer_and_rect.rect));
            tile->setPosition(x * 32, y * 32);
            u32 id = tileIDfromCoords(x * 32, y * 32);
            tilemap[id] = tile.get();

            if (layer_and_rect.layer == LAYER_MID) {
                tile->body = createBox(world, x * 32, y * 32, 32, 32,
                                       b2_staticBody, tile.get());
                m_layerNodes[LAYER_MID]->attachChild(std::move(tile));
            } else if (layer_and_rect.layer == LAYER_BACK) {
                tile->body = createBox(world, x * 32, y * 32, 32, 32,
                                       b2_staticBody, tile.get());
                tile->body->SetActive(false);
                m_layerNodes[LAYER_BACK]->attachChild(std::move(tile));
            }
        }
    }

    assert(playercount == 1);
}

b2Body* World::createBox(b2World& world, i32 posX, i32 posY, i32 sizeX,
                         i32 sizeY, b2BodyType type, void* userData)
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

    b2Body* body = world.CreateBody(&bodyDef);
    body->CreateFixture(&fixtureDef);

    body->SetUserData(userData);

    return body;
}

void World::spawnBox(sf::Vector2f pos, b32 isStatic)
{
    u32 tilenr = 5;
    b2BodyType type = b2_dynamicBody;
    if (isStatic)
    {
        type = b2_staticBody;
        tilenr = 2;
    }

    // clamp boxes to tileraster
    auto xpos = std::round(pos.x / 32) * 32;
    auto ypos = std::round(pos.y / 32) * 32;

    // there already is a tile here so don't spawn another one
    if (tilemap.find(tileIDfromCoords(xpos, ypos)) != tilemap.end()) return;

    std::unique_ptr<SpriteNode> box(new SpriteNode(m_textures.get(tiletexfile),
                                                   sf::IntRect(tilenr * 32, 0  * 32,
                                                               32, 32)));
    box->moving = true;
    box->shouldDraw = true;

    box->setPosition(xpos, ypos);
    box->typeflags = ENTITY_TILE | ENTITY_ENEMY;

    box->body = createBox(world, xpos, ypos, 32, 32, type, box.get());
    box->body->SetFixedRotation(true);
    m_layerNodes[LAYER_MID]->attachChild(std::move(box));
}

u32 tileIDfromCoords(const u32 x, const u32 y)
{
    return (x * maxMapSize.x) + y;
}
