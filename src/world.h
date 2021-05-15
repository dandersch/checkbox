#pragma once
#include "pch.h"
#include "resourcepool.h"
#include "entity.h"
#include "command.h"
#include "physics.h"

#include "levelgen.h"

// forward declarations
class Player;

class World : private sf::NonCopyable
{
public:
    explicit World(const std::string& levelName = "level1.png",
                   b32 menuMode = false);
    void update(f32 dt, sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    void spawnBox(sf::Vector2f pos, b32 isStatic = false);

    inline void handleInput() { m_player->handleInput(cmdQueue); }
    inline void handleEvents(sf::Event event)
    {
        m_player->handleEvent(event, cmdQueue);
    }

    void processEvent(const Event& evn);

public:
    sf::View m_view; // TODO private
    std::queue<Command> cmdQueue;
    std::vector<u32> cmdSerializer;
    Player* m_player; // TODO private

private:
    // physics
    PlayerContactListener playerTileContact;

    // TODO pass resourcepool / make resourcepool return singleton
    ResourcePool<sf::Texture> m_textures;
    ResourcePool<sf::Image> m_levels;

    Entity m_scenegraph;
    std::array<Entity*, LAYER_COUNT> m_layerNodes;
    b2World world; // TODO keep using higher gravity (?)
    sf::Vector2u maxMapSize;
    b2WheelJoint* hold_joint = nullptr;

    std::map<u32, Tile*> tilemap_mid;
    std::map<u32, Tile*> tilemap_back;

    b32 menuMode;

    // demomode
    std::queue<Command> demoCmds;
};
