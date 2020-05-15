#include "game.h"

Game::Game()
  : m_window(sf::VideoMode(1280, 720), "SFML game")
  , m_view(sf::Vector2f(640.f, 360.f), sf::Vector2f(VIEW_HEIGHT, VIEW_WIDTH))
  , m_texs(".png")
  , m_sfxs(".ogg")
  , m_fonts(".ttf")
  , m_player(m_texs)
  , m_slave(Enemy::Type::Slave, m_texs)
  , m_skeleton(Enemy::Type::Skeleton, m_texs)
  , m_world(m_window)
  , m_cursor()
  , m_collBox()
  , m_text()
  , m_sfx()
  , m_music()
{

    m_window.setVerticalSyncEnabled(true); // Don't use
    // m_window.setFramerateLimit(60);     // both
    ImGui::SFML::Init(m_window);

    // hide os mouse cursor
    m_window.setMouseCursorVisible(false);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    m_player.speed = 2.f;
    m_player.body.setPosition(300, 400);
    m_slave.setPosition(200, 400);
    m_skeleton.setPosition(250, 400);

    m_cursor.setTexture(m_texs.get("cursor.png"));
    m_cursor.setTextureRect(sf::IntRect(144, 0, 72, 72));

    m_text.setFont(m_fonts.get("Boxy-Bold.ttf"));
    m_text.setString("Hello world");
    m_text.setCharacterSize(28); // in pixels, not points!
    m_text.setPosition(200, 200);

    // Collision testshape
    m_collBox.setSize(sf::Vector2f(100, 100));
    m_collBox.setPosition(400, 400);
    m_collBox.setTexture(&m_texs.get("stonefloor.png"));

    // AUDIO
    m_sfx.setBuffer(m_sfxs.get("foom.ogg"));
    m_sfx.setVolume(20.f);

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
        switch (event.type) {
        case (sf::Event::Closed): m_window.close(); break;

        case (sf::Event::Resized): {
            float aspectratio = float(m_window.getSize().x) /
                                float(m_window.getSize().y);
            m_view.setSize(VIEW_HEIGHT * aspectratio, VIEW_HEIGHT);
            break;
        }

        case (sf::Event::MouseButtonPressed):
            m_cursor.setTextureRect(sf::IntRect(72, 0, 72, 72));
            break;

        case (sf::Event::MouseButtonReleased):
            m_cursor.setTextureRect(sf::IntRect(144, 0, 72, 72));
            break;

        case (sf::Event::MouseWheelScrolled): {
            // ZOOMING
            int scrolltick = event.mouseWheelScroll.delta;
            if (scrolltick == 1) m_view.zoom(0.9f);
            if (scrolltick == -1) m_view.zoom(1.1f);
            break;
        }

        default: break;
        }
    }
}

void Game::update(float dtime)
{
    m_view.setCenter(m_player.body.getPosition());
    m_player.update(dtime);

    // collision
    if (m_player.body.getGlobalBounds().intersects(m_collBox.getGlobalBounds())) {
        m_player.body.move(-m_player.velocity);
        m_sfx.play();
    }

    // TODO use cursor class (?)
    // convert mousepos to world coordinates
    m_cursor.setPosition(m_window.mapPixelToCoords(sf::Vector2i(sf::Mouse::getPosition(m_window).x - 9,
                                                                sf::Mouse::getPosition(m_window).y - 4)));

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        // lerp player to mousepos (wip)
        m_player.body.setPosition(m_player.body.getPosition() * (1.0f - dtime) +
                                  m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)) * dtime);
    }
}

void Game::render()
{
    m_window.clear(sf::Color(100, 180, 120, 255));

    m_world.draw();

    m_window.setView(m_view);

    m_window.draw(m_player);
    m_window.draw(m_collBox);
    m_window.draw(m_slave);
    m_window.draw(m_skeleton);

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
    ImGui::SliderFloat("player speed", &m_player.speed, -10, 10);
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
