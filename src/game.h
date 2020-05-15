#pragma once

#include "pch.h"

#include "player.h"
#include "resourcepool.h"

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
    sf::RenderWindow m_window;
    sf::View m_view;

    Player m_player;
    sf::Texture m_playerTexture;
    sf::Texture m_cursorTexture;
    sf::Sprite m_cursor;

    sf::RectangleShape m_collBox;
    sf::Music m_music;
    sf::Font m_font;
    sf::Text m_text;

    sf::SoundBuffer m_sfxBuffer;
    sf::Sound m_sfx;

    ResourcePool<sf::Texture> m_texs;
    // GameState m_state;
};
