#pragma once
#include "pch.h"
#include "player.h"
#include "resourcepool.h"
#include "entity.h"
#include "world.h"

#define VIEW_HEIGHT 1280
#define VIEW_WIDTH  720

enum GameState
{
    MAIN_MENU,
    IN_GAME,
    EXIT_GAME
};

class Game
{
public:
    Game();
    void run();

private:
    void processEvents();
    void update(f32 dtime);
    void render();
    void debugGui(sf::Time time);

public:
    GameState currentState = MAIN_MENU;

private:
    World m_world;

    // rendering
    sf::RenderWindow m_window;

    // resources
    ResourcePool<sf::Texture> m_texs;
    ResourcePool<sf::Font> m_fonts;
    //ResourcePool<sf::SoundBuffer> m_sfxs;

    sf::Texture m_cursorTexture;
    sf::Sprite m_cursor;
    sf::Text m_text;
    sf::Music m_music;
};
