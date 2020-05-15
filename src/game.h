#pragma once

#include "pch.h"

#include "player.h"
#include "resourcepool.h"
#include "entity.h"
#include "world.h"

#define VIEW_HEIGHT 1280
#define VIEW_WIDTH  720

class Game
{
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dtime);
    void render();
    void debugGui(sf::Time time);

    // TODO
    // void handleInput(sf::Keyboard::Key key, bool pressed);

private:
    // rendering
    sf::RenderWindow m_window;
    sf::View m_view;

    // resources
    ResourcePool<sf::Texture> m_texs;
    ResourcePool<sf::SoundBuffer> m_sfxs;
    ResourcePool<sf::Font> m_fonts;

    World m_world;

    Player m_player;
    Enemy m_slave;
    Enemy m_skeleton;

    sf::Texture m_cursorTexture;
    sf::Sprite m_cursor;
    sf::RectangleShape m_collBox;
    sf::Text m_text;
    sf::Sound m_sfx;
    sf::Music m_music;

    // GameState m_state;
};
