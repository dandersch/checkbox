#pragma once

#include "pch.h"
#include "resourcepool.h"
#include "scenenode.h"
#include "entity.h"

// forward declarations
class SpriteNode;
class Player;

class World : private sf::NonCopyable
{
public:
    explicit World(sf::RenderWindow& window);
    void update(float dt);
    void draw();

private:
    void loadTextures();
    void buildScene();

private:
    enum Layer
    {
        Background,
        Middle,
        Foreground,
        LayerCount
    };

    sf::RenderWindow& m_window;
    sf::View m_view;

    ResourcePool<sf::Texture> m_textures;
    //ResourcePool<sf::SoundBuffer> m_sfxs;
    //ResourcePool<sf::Font> m_fonts;

    SceneNode m_scenegraph;
    std::array<SceneNode*, LayerCount> m_layerNodes;
    sf::Vector2f m_playerSpawn;
    Player* m_player;

    SpriteNode* box;
};
