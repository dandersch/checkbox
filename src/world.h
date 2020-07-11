#pragma once
#include "pch.h"
#include "resourcepool.h"
#include "entity.h"
#include "command.h"
#include "physics.h"

// forward declarations
class Player;

class World : private sf::NonCopyable
{
public:
    explicit World(sf::RenderWindow& window);
    void update(f32 dt);
    void draw();
    void spawnBox(sf::Vector2f pos, b32 isStatic = false);

public:
    sf::View m_view; // TODO private
    std::queue<Command> cmdQueue;
    Player* m_player; // TODO private

private:
    sf::RenderWindow& m_window;

    // physics
    PlayerContactListener playerTileContact;
};
