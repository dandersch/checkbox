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
    if (!m_player->body.getGlobalBounds().intersects(box->m_sprite.getGlobalBounds()))
        // m_view.setCenter(m_player->body.getPosition());
        m_view.move(m_player->velocity);

    //box->setPosition(m_player->body.getPosition());
    box->m_sprite.setPosition(m_window.mapPixelToCoords(sf::Vector2i(640, 320), m_view));
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

    std::unique_ptr<SpriteNode> box(new SpriteNode(m_textures.get("simple_"
                                                                  "tiles_32x32."
                                                                  "png")));
    this->box = box.get();
    m_layerNodes[Background]->attachChild(std::move(box));

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
