#pragma once

#include "pch.h"
#include "resourcepool.h"
#include "scenenode.h"
#include "entity.h"
#include "command.h"
#include "physics.h"

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

    b2Body* createBox(b2World& world, int posX, int posY, int sizeX, int sizeY,
                      b2BodyType type, void* userData);

public:
    sf::View m_view; // TODO private
    std::queue<Command> cmdQueue;
    Player* m_player; // TODO private

private:
    enum Layer
    {
        Background,
        Middle,
        Foreground,
        LayerCount
    };

    sf::RenderWindow& m_window;

    ResourcePool<sf::Texture> m_textures;
    ResourcePool<sf::Image> m_levels;
    //ResourcePool<sf::SoundBuffer> m_sfxs;
    //ResourcePool<sf::Font> m_fonts;

    // physics
    b2World world;
    PlayerContactListener playerTileContact;

    SceneNode m_scenegraph;
    std::array<SceneNode*, LayerCount> m_layerNodes;
};
