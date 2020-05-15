#include "world.h"
#include "spritenode.h"

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
}
