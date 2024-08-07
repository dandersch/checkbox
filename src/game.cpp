#include "game.h"
#include "menu.h"

std::vector<std::function<void(void)>> g_gui_callbacks;
static const sf::Time TIME_PER_FRAME = sf::seconds(1.f / 60.f);
// for FPS
static sf::Clock fps_clock;
static f32 fps_last_time = 0;
static f32 fps = 0;
// for ticks per second
static sf::Clock tps_clock;
static f32 tps_last_time = 0;
static f32 tps = 0;

static bool show_debug = false;

Game::Game()
  : m_window(sf::VideoMode(1920, 1080), "SFML game")
  , m_texs(".png")
  , m_fonts(".ttf")
  , m_cursor(m_texs.get("cursor.png"), sf::IntRect(144, 0, 72, 72))
  , m_text("", m_fonts.get("Boxy-Bold.ttf"), 70)
  , m_music()
  , m_world("level_new.png")
{
    m_window.setVerticalSyncEnabled(true); // Don't use
    // m_window.setFramerateLimit(60);     // both
    ImGui::SFML::Init(m_window);

    // hide os mouse cursor
    m_window.setMouseCursorVisible(false);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    m_text.setPosition(300, 800);

    m_music.openFromFile("./res/nescaline.wav");
    m_music.setVolume(100.f);
    m_music.setLoop(true);
    m_music.play();

    menuInit(this, m_texs, m_fonts);
}

void Game::processEvents()
{
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (show_debug) ImGui::SFML::ProcessEvent(event);

        switch (currentState)
        {
        case MAIN_MENU: menuHandleEvents(event); break;
        case IN_GAME: m_world.handleEvents(event); break;
        case EXIT_GAME: break;
        }

        switch (event.type) {
        case (sf::Event::Closed):
        {
            // serialize commands
            /*
            std::ofstream democommands;
            democommands.open("democommands.txt");
            for (auto i : m_world.cmdSerializer) democommands << i;
            democommands.close();
            */
            m_window.close();
            break;
        }

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
                m_world.spawnBox(pos, false);
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

        case (sf::Event::KeyPressed):
            if (event.key.code == sf::Keyboard::F1) show_debug = !show_debug;

        default: break;
        }
    }

    switch (currentState)
    {
    case MAIN_MENU: menuHandleInput(); break;
    case IN_GAME: m_world.handleInput(); break;
    case EXIT_GAME: break;
    }
}

void Game::update(f32 dtime)
{
    switch (currentState)
    {
    case MAIN_MENU:
    {
        menuWorldUpdate(dtime, m_window);
        break;
    }
    case IN_GAME:
    {
        m_world.update(dtime, m_window);
        // TODO(dan): testing lifecount
        auto coins = m_world.m_player->goldCount;
        m_text.setFillColor(sf::Color(238,205,1));
        m_text.setString(std::to_string(coins));

        if (m_world.m_player->gameOver)
        {
            m_world.m_player->gameOver = false;
            currentState = MAIN_MENU;
        }
        break;
    }
    case EXIT_GAME:
    {
        m_window.close();
        break;
    }
    }

    // TODO use cursor class (?)
    // convert mousepos to world coordinates
    m_cursor.setPosition(m_window.mapPixelToCoords(sf::Vector2i(sf::Mouse::getPosition(m_window).x - 9,
                                                                sf::Mouse::getPosition(m_window).y - 4)));
}

void Game::render()
{
    f32 current_time = fps_clock.restart().asSeconds();
    fps = 1.f / current_time;
    fps_last_time = current_time;

    m_window.clear(sf::Color(140, 170, 200, 255));

    switch (currentState)
    {
    case MAIN_MENU:
    {
        menuDraw(m_window);
        break;
    }
    case IN_GAME:
    {
        m_world.draw(m_window);

        // Everything drawn after setting the view will appear fixed on the
        // screen
        m_window.setView(m_window.getDefaultView());
        {
            m_window.draw(m_text); // TODO(dan): testing text
        }
        m_window.setView(m_world.m_view);

        break;
    }
    case EXIT_GAME:
    {
        break;
    }
    }

    if (show_debug)
    {
        ImGui::SFML::Render(m_window);
        m_window.draw(m_cursor);
    }

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

    if (ImGui::Button("Reset View"))
        m_world.m_view.setSize(2560, 1440);

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

        if (show_debug) debugGui(accumulator);

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
