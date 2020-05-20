#include "world.h"
#include "spritenode.h"
#include "player.h"
#include "command.h"

#define VIEW_HEIGHT 1280
#define VIEW_WIDTH  720

static const char* tiletexfile = "simple_tiles_32x32.png";

World::World(sf::RenderWindow& window)
  : m_window(window)
  , m_view(sf::Vector2f(640.f, 360.f), sf::Vector2f(VIEW_HEIGHT, VIEW_WIDTH))
  , m_textures(".png")
  , m_levels(".png")
  , m_player(nullptr)
    //, world(b2Vec2(0.f, 9.81f))
  , world(b2Vec2(0.f, 25.f)) // TODO keep using higher gravity (?)
{
    loadTextures();
    buildScene();

    m_player->body = createBox(world, m_player->getPosition().x,
                               m_player->getPosition().y, 64, 64,
                               b2_dynamicBody, m_player);
    m_player->body->SetFixedRotation(true);

    //at global scope
    world.SetContactListener(&playerTileContact);
}

void World::update(float dt)
{
    // VIEW UPDATE /////////////////////////////////////////////////////////////
    // get player position on screen in pixels
    sf::Vector2u pPixelPos = (sf::Vector2u)m_window.mapCoordsToPixel(m_player->getPosition(),
                                                                     m_view);
    // get player position on screen in [0-1]
    sf::Vector2f pRelPos((float)pPixelPos.x / m_window.getSize().x,
                         (float)pPixelPos.y / m_window.getSize().y);

    // move view if player gets out of borders
    // TODO breaks if player near border and zooming in/resizing
    if (pRelPos.x < 0.3f && m_player->velocity.x < 0.f) {
        m_view.move(m_player->velocity.x * dt, 0.f);
        m_view.setCenter(std::floor(m_view.getCenter().x), m_view.getCenter().y);
    }
    if (pRelPos.x > 0.7f && m_player->velocity.x > 0.f) {
        m_view.move(m_player->velocity.x * dt, 0.f);
        m_view.setCenter(std::ceil(m_view.getCenter().x), m_view.getCenter().y);
    }
    if (pRelPos.y < 0.3f && m_player->velocity.y < 0.f) {
        m_view.move(0.f, m_player->velocity.y * dt);
        m_view.setCenter(m_view.getCenter().x, std::ceil(m_view.getCenter().y));
    }
    if (pRelPos.y > 0.7f && m_player->velocity.y > 0.f) {
        m_view.move(0.f, m_player->velocity.y * dt);
        m_view.setCenter(m_view.getCenter().x, std::floor(m_view.getCenter().y));
    }

    // workaround to let player not get stuck outside of viewborders
    if (pRelPos.x < 0.28f) m_view.setCenter(m_player->getPosition());
    if (pRelPos.x > 0.72f) m_view.setCenter(m_player->getPosition());
    if (pRelPos.y < 0.28f) m_view.setCenter(m_player->getPosition());
    if (pRelPos.y > 0.72f) m_view.setCenter(m_player->getPosition());
    ////////////////////////////////////////////////////////////////////////////

    //m_view.setCenter(std::round(m_view.getCenter().x), std::round(m_view.getCenter().y));

    // reset velocity
    m_player->velocity.x = 0.f;
    //m_player->velocity.y = 0.f;

    while (!cmdQueue.empty()) {
        Command cmd = cmdQueue.front();
        cmdQueue.pop();
        m_scenegraph.onCommand(cmd, dt);
    }

    // set player velocity
    m_player->velocity.y += metersToPixels(world.GetGravity().y) * dt;
    m_player->body->SetLinearVelocity(b2Vec2(pixelsToMeters(m_player->velocity.x),
                                             pixelsToMeters(m_player->velocity.y)));
    world.Step(dt, int32(8), int32(3)); // physics update
    m_scenegraph.update(dt);

    if (m_player->velocity.y > 2000.f) // workaround
        m_player->velocity.y = 2000.f;

    /*
    // aabb collision detection & response for player
    auto playerCollider = m_player->getBoundingRect();
    collisionInfo cinfo;
    m_scenegraph.checkCollisions(playerCollider, cinfo);

    // GRAVITY
    // TODO builds up and breaks things over time
    if (!cinfo.touchingGround)
        m_player->velocity.y += 981.f * dt;
    else // workaround
        m_player->velocity.y = 10.f;

    if (m_player->velocity.y > 2000.f) // workaround
        m_player->velocity.y = 2000.f;

    m_player->canJump = cinfo.touchingGround;
    m_player->setPosition(playerCollider.left + playerCollider.width / 2,
                          playerCollider.top + playerCollider.height / 2);
    */


}

void World::draw()
{
    m_window.setView(m_view);
    m_window.draw(m_scenegraph);
}

void World::loadTextures()
{
    // sort of a "preload"
    m_textures.get("stonefloor_512x512.png");
}

void World::buildScene()
{
    // initialize nodes for every layer of the scene
    for (std::size_t i = 0; i < LayerCount; i++) {
        std::unique_ptr<SceneNode> layer(new SceneNode());
        m_layerNodes[i] = layer.get();
        m_scenegraph.attachChild(std::move(layer));
    }

//  auto& bgTex = m_textures.get("stonefloor_512x512.png");
//  sf::IntRect bgTexRect(0,0,8192,8192);
//  bgTex.setRepeated(true);
//  std::unique_ptr<SpriteNode> bgSprite(new SpriteNode(bgTex, bgTexRect));
//  bgSprite->setPosition(-4096, -4096);
//  m_layerNodes[Background]->attachChild(std::move(bgSprite));

    std::unique_ptr<Player> player(new Player(m_textures));
    m_player = player.get();
    m_view.setCenter(m_player->getPosition());
    m_layerNodes[Middle]->attachChild(std::move(player));

    //////////////////////////////////////////////////
    // Generation of level from image:
    //////////////////////////////////////////////////

    // lambda comparator to be able to create a map with sf::Color as key
    auto comparator = [](const sf::Color& c1, const sf::Color& c2) -> bool {
        if (c1.r < c2.r) return true;
        else if (c1.r == c2.r && c1.g < c2.g) return true;
        else if (c1.r == c2.r && c1.g == c2.g && c1.b < c2.b) return true;
        return false;
    };

    std::map<sf::Color, sf::IntRect, decltype(comparator)> colorMap(comparator);

    auto& levelTex = m_textures.get(tiletexfile);
    auto& level0 = m_levels.get("level0.png");

    // fill the colormap
    int xCount = levelTex.getSize().x / 32;
    int yCount = levelTex.getSize().y / 32;

    for (int y = 0; y < yCount; y++) {
        for (int x = 0; x < xCount; x++) {
            colorMap[level0.getPixel(x, y)] = sf::IntRect(x * 32, y * 32, 32, 32);
            level0.setPixel(x, y, sf::Color::White);
        }
    }

    for (int y = 0; y < level0.getSize().y; y++) {
        for (int x = 0; x < level0.getSize().x; x++) {
            sf::Color sample = level0.getPixel(x,y);

            if (sample == sf::Color::Red) {
                m_player->setPosition(x * 32, y * 32);
                continue;
            }

            if (sample == sf::Color::White) continue; // whitespace

            std::unique_ptr<SpriteNode> tile(new SpriteNode(levelTex,
                                                            colorMap[sample]));

            tile->setPosition(x * 32, y * 32);
            tile->body = createBox(world, x * 32, y * 32, 32, 32, b2_staticBody,
                                   tile.get());
            m_layerNodes[Foreground]->attachChild(std::move(tile));
        }
    }

    std::unique_ptr<SpriteNode> box(new SpriteNode(levelTex, sf::IntRect(0,0,32,32)));
    box->body = createBox(world, 18 * 32, 9 * 32, 32 , 32, b2_dynamicBody, box.get());
    m_layerNodes[Foreground]->attachChild(std::move(box));
    std::unique_ptr<SpriteNode> box2(new SpriteNode(levelTex, sf::IntRect(0,0,32,32)));
    box2->body = createBox(world, 18 * 32, 9 * 32, 32 , 32, b2_dynamicBody, box2.get());
    box2->body->SetFixedRotation(true);
    m_layerNodes[Foreground]->attachChild(std::move(box2));
}

b2Body* World::createBox(b2World& world, int posX, int posY, int sizeX,
                         int sizeY, b2BodyType type, void* userData)
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

void World::spawnBox(sf::Vector2f pos, bool isStatic)
{
    b2BodyType type = b2_dynamicBody;
    if (isStatic) type = b2_staticBody;

    std::unique_ptr<SpriteNode> box(new SpriteNode(m_textures.get(tiletexfile),
                                                   sf::IntRect(2 * 32, 0 * 32,
                                                               32, 32)));
    box->setPosition((int)pos.x, (int)pos.y);

    box->body = createBox(world, (int) pos.x, (int) pos.y, 32, 32, type, box.get());
    m_layerNodes[Foreground]->attachChild(std::move(box));
}
