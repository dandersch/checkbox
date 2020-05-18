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
{
    loadTextures();
    buildScene();
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
    if (pRelPos.x < 0.2f) m_view.move(m_player->velocity * dt);
    if (pRelPos.x > 0.8f) m_view.move(m_player->velocity * dt);
    if (pRelPos.y < 0.2f) m_view.move(m_player->velocity * dt);
    if (pRelPos.y > 0.8f) m_view.move(m_player->velocity * dt);

    // workaround to let player not get stuck outside of viewborders
    if (pRelPos.x < 0.19f) m_view.setCenter(m_player->getPosition());
    if (pRelPos.x > 0.81f) m_view.setCenter(m_player->getPosition());
    if (pRelPos.y < 0.19f) m_view.setCenter(m_player->getPosition());
    if (pRelPos.y > 0.81f) m_view.setCenter(m_player->getPosition());
    ////////////////////////////////////////////////////////////////////////////

    // reset velocity
    m_player->velocity.x = 0.f;
    //m_player->velocity.y = 0.f;

    // TODO builds up and breaks things over time
    m_player->velocity.y += 981.f * dt; // Gravity
    if (m_player->velocity.y > 2000.f) // workaround
        m_player->velocity.y = 2000.f;

    while (!cmdQueue.empty()) {
        Command cmd = cmdQueue.front();
        cmdQueue.pop();
        m_scenegraph.onCommand(cmd, dt);
    }

    m_scenegraph.update(dt);

    // aabb collision detection & response for player
    // TODO breaks when colliding with multiple boxes
    auto playerCollider = m_player->getBoundingRect();
    collisionInfo cinfo;
    m_scenegraph.checkCollisions(playerCollider, cinfo);

    if (cinfo.collided) {
        float intersectx = std::abs(cinfo.movement.width) -
                           (cinfo.halfsize.x + (playerCollider.width / 2));
        float intersecty = std::abs(cinfo.movement.height) -
                           (cinfo.halfsize.y + (playerCollider.height / 2));
        if (intersectx < intersecty) {
            // x-collision
            if (cinfo.movement.left > cinfo.position.x)
                m_player->move(cinfo.movement.width, 0.f);
            else
                m_player->move(-cinfo.movement.width, 0.f);
        } else {
            // y-collision
            if (cinfo.movement.top < cinfo.position.y) {
                m_player->move(0.f, -cinfo.movement.height);
                m_player->canJump = true;
            }
            else {
                m_player->move(0.f, cinfo.movement.height);
            }
        }
    }
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

    auto& bgTex = m_textures.get("stonefloor_512x512.png");
    sf::IntRect bgTexRect(0,0,8192,8192);
    bgTex.setRepeated(true);
    std::unique_ptr<SpriteNode> bgSprite(new SpriteNode(bgTex, bgTexRect));
    bgSprite->setPosition(-4096, -4096);
    m_layerNodes[Background]->attachChild(std::move(bgSprite));

    std::unique_ptr<Player> player(new Player(m_textures));
    m_player = player.get();
    m_player->body.setOrigin(32,32);
    m_view.setCenter(m_player->getPosition());
    m_layerNodes[Middle]->attachChild(std::move(player));

    std::unique_ptr<SpriteNode> test(new SpriteNode(m_textures.get("as"), sf::IntRect(0,0,300,300)));
    test->setPosition(5 * 32, 7 * 32);
    m_layerNodes[Foreground]->attachChild(std::move(test));

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
            m_layerNodes[Foreground]->attachChild(std::move(tile));
        }
    }

    m_player->setPosition(3 * 32, 0); // for testing
}
