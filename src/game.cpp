#include "game.h"

Game::Game()
  : m_window(sf::VideoMode(1280, 720), "SFML game")
  , m_texs(".png")
  , m_fonts(".ttf")
  , m_world(m_window)
  , m_cursor()
  , m_text()
  , m_music()
{
    m_window.setVerticalSyncEnabled(true); // Don't use
    // m_window.setFramerateLimit(60);     // both
    ImGui::SFML::Init(m_window);

    // hide os mouse cursor
    m_window.setMouseCursorVisible(false);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    m_cursor.setTexture(m_texs.get("cursor.png"));
    m_cursor.setTextureRect(sf::IntRect(144, 0, 72, 72));

    m_text.setFont(m_fonts.get("Boxy-Bold.ttf"));
    m_text.setString("Hello World");
    m_text.setCharacterSize(28); // in pixels, not points!
    m_text.setPosition(100, 900);

    m_music.openFromFile("../res/intro.ogg");
    m_music.setLoop(true);
    m_music.play();
    m_music.setVolume(5.f);
}

void Game::processEvents()
{
    sf::Event event;
    while (m_window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        m_world.m_player->handleEvent(event, m_world.cmdQueue);

        switch (event.type) {
        case (sf::Event::Closed): m_window.close(); break;

        case (sf::Event::Resized): {
            float aspectratio = float(m_window.getSize().x) /
                                float(m_window.getSize().y);
            // TODO breaks pixelperfect rendering
            //m_world.m_view.setSize(VIEW_HEIGHT * aspectratio, VIEW_HEIGHT);
            break;
        }

        case (sf::Event::MouseButtonPressed): {
            m_cursor.setTextureRect(sf::IntRect(72, 0, 72, 72));
            auto pos = m_window.mapPixelToCoords(sf::Vector2i(sf::Mouse::getPosition(m_window).x - 9,
                                                              sf::Mouse::getPosition(m_window).y - 4));
            if (event.key.code == sf::Mouse::Left)
                m_world.spawnBox(pos);
            if (event.key.code == sf::Mouse::Right)
                m_world.spawnBox(pos, true);
            break;
        }

        case (sf::Event::MouseButtonReleased):
            m_cursor.setTextureRect(sf::IntRect(144, 0, 72, 72));
            break;

        case (sf::Event::MouseWheelScrolled): {
            // ZOOMING
            int scrolltick = event.mouseWheelScroll.delta;
            // TODO breaks pixelperfect rendering
            if (scrolltick == 1)
                m_world.m_view.setSize((int)m_world.m_view.getSize().x / 2,
                                       (int)m_world.m_view.getSize().y / 2);
            if (scrolltick == -1)
                m_world.m_view.setSize((int)m_world.m_view.getSize().x * 1.5f,
                                       (int)m_world.m_view.getSize().y * 1.5f);
            break;
        }

        default: break;
        }

    }
    m_world.m_player->handleInput(m_world.cmdQueue);
}

void Game::update(float dtime)
{
    m_world.update(dtime);

    // TODO use cursor class (?)
    // convert mousepos to world coordinates
    m_cursor.setPosition(m_window.mapPixelToCoords(sf::Vector2i(sf::Mouse::getPosition(m_window).x - 9,
                                                                sf::Mouse::getPosition(m_window).y - 4)));
    /*
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        // lerp player to mousepos (wip)
        m_player.body.setPosition(m_player.body.getPosition() * (1.0f - dtime) +
                                  m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)) * dtime);
    }
    */
}

void Game::render()
{
    m_window.clear(sf::Color(140, 170, 200, 255));
    m_world.draw();

    // Everything drawn after setting the view will appear fixed on the screen
    //m_window.setView(m_window.getDefaultView());
    //std::cout << m_window.getDefaultView().getCenter().x << std::endl;
    //std::cout << m_window.getDefaultView().getCenter().y << std::endl;

    m_window.draw(m_text);
    ImGui::SFML::Render(m_window);
    m_window.draw(m_cursor);
    m_window.display();
}

// can't be called more than once before render()
void Game::debugGui(sf::Time time)
{
    ImGui::SFML::Update(m_window, time);
    ImGui::Begin("Hello");
    //ImGui::SliderFloat("player speed", &m_player.speed, -10, 10);
    if (ImGui::Button("Pause"))
        m_music.getStatus() == m_music.Paused ? m_music.play()
                                              : m_music.pause();
    ImGui::End();
}

static const sf::Time TIME_PER_FRAME = sf::seconds(1.f/60.f);
void Game::run()
{
    sf::Clock clock; // starts the clock
    sf::Time accumulator = sf::Time::Zero;

    while (m_window.isOpen()) {
        processEvents();
        accumulator += clock.restart();

        debugGui(accumulator);

        while (accumulator > TIME_PER_FRAME) {
            accumulator -= TIME_PER_FRAME;
            processEvents();
            update(TIME_PER_FRAME.asSeconds());
        }
        render();
    }
}
