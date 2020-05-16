#include "world.h"
#include "spritenode.h"

#include "player.h"

#define VIEW_HEIGHT 1280
#define VIEW_WIDTH  720

static const char* tiletexfile = "simple_tiles_32x32.png";

World::World(sf::RenderWindow& window)
  : m_window(window)
  , m_view(sf::Vector2f(640.f, 360.f), sf::Vector2f(VIEW_HEIGHT, VIEW_WIDTH))
  , m_textures(".png")
  , m_levels(".png")
  , m_playerSpawn(300, 400)
  , m_player(nullptr)
{
    // loadTextures();
    buildScene();
    //m_view.setCenter(m_playerSpawn);
}

void World::update(float dt)
{
    // VIEW UPDATE ///
    // get player position on screen in pixels
    sf::Vector2u pPixelPos = (sf::Vector2u)m_window.mapCoordsToPixel(m_player->body.getPosition(),
                                                                     m_view);
    // get player position on screen in [0-1]
    sf::Vector2f pRelPos((float)pPixelPos.x / m_window.getSize().x,
                         (float)pPixelPos.y / m_window.getSize().y);

    // move view if player gets out of borders
    // TODO breaks if player near border and zooming in/resizing
    if (pRelPos.x < 0.2f) m_view.move(m_player->velocity);
    if (pRelPos.x > 0.8f) m_view.move(m_player->velocity);
    if (pRelPos.y < 0.2f) m_view.move(m_player->velocity);
    if (pRelPos.y > 0.8f) m_view.move(m_player->velocity);

    // workaround to let player not get stuck outside of viewborders
    if (pRelPos.x < 0.19f) m_view.setCenter(m_player->body.getPosition());
    if (pRelPos.x > 0.81f) m_view.setCenter(m_player->body.getPosition());
    if (pRelPos.y < 0.19f) m_view.setCenter(m_player->body.getPosition());
    if (pRelPos.y > 0.81f) m_view.setCenter(m_player->body.getPosition());

    m_scenegraph.update(dt);
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
    m_player->speed = 2.f;
    m_view.setCenter(m_player->body.getPosition());
    m_layerNodes[Middle]->attachChild(std::move(player));

    // Generation of map as image:
    auto& lvlgentex = m_textures.get(tiletexfile);

    // lambda comparator to be able to create a map with sf::Color as key
    auto comparator = [](const sf::Color& c1, const sf::Color& c2) -> bool {
        if (c1.r < c2.r) return true;
        else if (c1.r == c2.r && c1.g < c2.g) return true;
        else if (c1.r == c2.r && c1.g == c2.g && c1.b < c2.b) return true;
        return false;
    };

    std::map<sf::Color, sf::IntRect, decltype(comparator)> colorMap(comparator);
    colorMap =
        { { sf::Color(255, 255, 255), sf::IntRect() },  // empty
          { sf::Color(255,   0, 255), sf::IntRect() },  // playerpos
          { sf::Color(  0, 255,   0), sf::IntRect(6 * 32, 0 * 32, 32, 32) },  // grass
          { sf::Color(  0,   0, 255), sf::IntRect(8 * 32, 0 * 32, 32, 32) },  // water
          { sf::Color(255, 255,   0), sf::IntRect(3 * 32, 0 * 32, 32, 32) },  // wood
          { sf::Color(255, 120,   0), sf::IntRect(2 * 32, 0 * 32, 32, 32) },  // rooftile
          { sf::Color(100,  60,   0), sf::IntRect(1 * 32, 0 * 32, 32, 32) },  // dirt
          { sf::Color(100, 100, 100), sf::IntRect(0 * 32, 0 * 32, 32, 32) }   // stone
    };

    auto& level0 = m_levels.get("level0.png");
    for (int y = 0; y < level0.getSize().y; y++) {
        for (int x = 0; x < level0.getSize().x; x++) {
            sf::Color sample = level0.getPixel(x,y);

            if (sample == sf::Color::Red) {
                m_player->body.setPosition(x * 32, y * 32);
                continue;
            }

            if (sample == sf::Color::White) continue; // whitespace

            std::unique_ptr<SpriteNode> tile(new SpriteNode(lvlgentex,
                                                            colorMap[sample]));

            tile->setPosition(x * 32, y * 32);
            m_layerNodes[Foreground]->attachChild(std::move(tile));
        }
    }
}
