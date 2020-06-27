#pragma once
#include "pch.h"
#include "resourcepool.h"
#include "entity.h"
#include "command.h"
#include "physics.h"

// forward declarations
class Player;

enum Layer
{
    LAYER_BACK,
    LAYER_MID,
    LAYER_FORE,
    LAYER_COUNT
};

class World : private sf::NonCopyable
{
public:
    explicit World(sf::RenderWindow& window);
    void update(f32 dt);
    void draw();
    void spawnBox(sf::Vector2f pos, b32 isStatic = false);

private:
    void loadTextures();
    void buildScene();

    b2Body* createBox(b2World& world, i32 posX, i32 posY, i32 sizeX, i32 sizeY,
                      b2BodyType type, void* userData);

public:
    sf::View m_view; // TODO private
    std::queue<Command> cmdQueue;
    Player* m_player; // TODO private

private:
    sf::RenderWindow& m_window;

    ResourcePool<sf::Texture> m_textures;
    ResourcePool<sf::Image> m_levels;
    //ResourcePool<sf::SoundBuffer> m_sfxs;
    //ResourcePool<sf::Font> m_fonts;

    // physics
    b2World world;
    PlayerContactListener playerTileContact;

    Entity m_scenegraph;
    std::array<Entity*, LAYER_COUNT> m_layerNodes;
};
