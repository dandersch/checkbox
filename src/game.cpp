#include "game.h"

std::vector<std::function<void(void)>> g_gui_callbacks;
static const sf::Time TIME_PER_FRAME = sf::seconds(1.f/60.f);
// for FPS
static sf::Clock fps_clock;
static f32 fps_last_time = 0;
static f32 fps = 0;
// for ticks per second
static sf::Clock tps_clock;
static f32 tps_last_time = 0;
static f32 tps = 0;

enum GameState
{
    MAIN_MENU,
    INGAME
};
static GameState currentState = MAIN_MENU;

Game::Game()
  : m_window(sf::VideoMode(1280, 720), "SFML game")
  , m_texs(".png")
  , m_fonts(".ttf")
  , m_world(m_window)
  , m_cursor(m_texs.get("cursor.png"), sf::IntRect(144, 0, 72, 72))
  , m_text("Hello World", m_fonts.get("Boxy-Bold.ttf"), 35)
  , m_music()
{
    m_window.setVerticalSyncEnabled(true); // Don't use
    // m_window.setFramerateLimit(60);     // both
    ImGui::SFML::Init(m_window);

    // hide os mouse cursor
    m_window.setMouseCursorVisible(false);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    m_text.setPosition(100, 900);

    m_music.openFromFile("../res/intro.ogg");
    m_music.setLoop(true);
    m_music.play();
    m_music.pause();
    m_music.setVolume(1.f);
}

void Game::processEvents()
{
    sf::Event event;
    while (m_window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        m_world.m_player->handleEvent(event, m_world.cmdQueue);

        switch (event.type) {
        case (sf::Event::Closed): m_window.close(); break;

        case (sf::Event::Resized):
        {
            f32 aspectratio = (f32) m_window.getSize().x /
                              (f32) m_window.getSize().y;
            // TODO breaks pixelperfect rendering
            // m_world.m_view.setSize(VIEW_HEIGHT * aspectratio, VIEW_HEIGHT);
            break;
        }

        case (sf::Event::MouseButtonPressed):
        {
            m_cursor.setTextureRect(sf::IntRect(72, 0, 72, 72));
            auto pos = m_window.mapPixelToCoords(sf::Vector2i(sf::Mouse::getPosition(m_window).x - 9,
                                                              sf::Mouse::getPosition(m_window).y - 4));
            if (event.key.code == (sf::Keyboard::Key) sf::Mouse::Left)
                m_world.spawnBox(pos);
            if (event.key.code == (sf::Keyboard::Key) sf::Mouse::Right)
                m_world.spawnBox(pos, true);
            break;
        }

        case (sf::Event::MouseButtonReleased):
            m_cursor.setTextureRect(sf::IntRect(144, 0, 72, 72));
            break;

        case (sf::Event::MouseWheelScrolled): {
            // ZOOMING
            i32 scrolltick = event.mouseWheelScroll.delta;
            // TODO breaks pixelperfect rendering
            if (scrolltick == 1)
            {
                m_world.m_view.setSize((i32) m_world.m_view.getSize().x / 2,
                                       (i32) m_world.m_view.getSize().y / 2);
                std::cout << "x: " << m_world.m_view.getSize().x << std::endl;
                std::cout << "y: " << m_world.m_view.getSize().y << std::endl;
            }
            if (scrolltick == -1)
            {
                m_world.m_view.setSize((i32) m_world.m_view.getSize().x * 1.5f,
                                       (i32) m_world.m_view.getSize().y * 1.5f);
                std::cout << "x: " << m_world.m_view.getSize().x << std::endl;
                std::cout << "y: " << m_world.m_view.getSize().y << std::endl;
            }
            break;
        }

        default: break;
        }
    }

    m_world.m_player->handleInput(m_world.cmdQueue);
}

void Game::update(f32 dtime)
{
    m_world.update(dtime);

    // TODO use cursor class (?)
    // convert mousepos to world coordinates
    m_cursor.setPosition(m_window.mapPixelToCoords(sf::Vector2i(sf::Mouse::getPosition(m_window).x - 9,
                                                                sf::Mouse::getPosition(m_window).y - 4)));
}

void Game::render()
{
    m_window.clear(sf::Color(140, 170, 200, 255));
    m_world.draw();

    f32 current_time = fps_clock.restart().asSeconds();
    fps = 1.f / current_time;
    fps_last_time = current_time;

    // Everything drawn after setting the view will appear fixed on the screen
    m_window.setView(m_window.getDefaultView());
    m_window.draw(m_text); // TODO(dan): testing text
    m_window.setView(m_world.m_view);

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
    ImGui::Text("FPS: %.3f", fps); ImGui::SameLine();
    ImGui::Text("TPS: %.3f", tps);

    for (auto func : g_gui_callbacks)
        func();

    ImGui::End();
}

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

            f32 current_time = tps_clock.restart().asSeconds();
            tps = 1.f / current_time;
            tps_last_time = current_time;
        }
        render();
    }
}
