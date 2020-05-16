#include "world.h"
#include "spritenode.h"

#include "player.h"

#define VIEW_HEIGHT 1280
#define VIEW_WIDTH  720

World::World(sf::RenderWindow& window)
  : m_window(window)
  , m_view(sf::Vector2f(640.f, 360.f), sf::Vector2f(VIEW_HEIGHT, VIEW_WIDTH))
  , m_textures(".png")
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
    sf::IntRect bgTexRect(0,0,2048,2048);
    bgTex.setRepeated(true);
    std::unique_ptr<SpriteNode> bgSprite(new SpriteNode(bgTex, bgTexRect));
    bgSprite->setPosition(0, 0);
    m_layerNodes[Background]->attachChild(std::move(bgSprite));

    std::unique_ptr<Player> player(new Player(m_textures));
    m_player = player.get();
    m_player->body.setPosition(300, 400);
    m_player->body.setOrigin(32,32);
    m_player->speed = 2.f;
    m_view.setCenter(m_player->body.getPosition());
    m_layerNodes[Middle]->attachChild(std::move(player));
}
