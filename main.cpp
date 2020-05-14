#include "pch.h"

#include "animation.h"
#include "player.h"

#define VIEW_HEIGHT 1280
#define VIEW_WIDTH  720

void findAssets(std::vector<std::string> exts,
                std::vector<std::string>& outfiles);
void resizeView(const sf::RenderWindow& window, sf::View& view);

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML game");
    sf::View view(sf::Vector2f(640.f, 360.f), sf::Vector2f(VIEW_HEIGHT, VIEW_WIDTH));

    window.setVerticalSyncEnabled(true); // Don't use
    // window.setFramerateLimit(60);     // both
    ImGui::SFML::Init(window);

    // hide os mouse cursor
    window.setMouseCursorVisible(false);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    bool show_debug = true;

    // FILESYSTEM STUFF
    std::vector<std::string> exts, musicfiles;
    exts.push_back(".ogg");
    //exts.push_back(".wav");
    findAssets(exts, musicfiles);

    // FONT & TEXT
    sf::Font font;
    font.loadFromFile("../assets/UbuntuMono-B.ttf");
    sf::Text text;
    text.setFont(font);
    text.setString("Hello world");
    text.setCharacterSize(24); // in pixels, not points!
    text.setFillColor(sf::Color::Red);
    // text.setStyle(sf::Text::Bold | sf::Text::Underlined);
    text.setPosition(200,200);

    // TEXTURE (8x9)
    int xstep = 64;
    int ystep = 64;
    sf::Texture playerTexture;
    if (!playerTexture.loadFromFile("../assets/platformer_sprites_base.png"))
        return -1;

    // Player
    Player player(&playerTexture);
    player.speed = 2.f;

    // Testshape
    sf::RectangleShape box;
    box.setSize(sf::Vector2f(100,100));
    box.setPosition(400, 400);
    box.setFillColor(sf::Color::Blue);

    // CURSOR
    sf::Texture cursorSheet;
    if (!cursorSheet.loadFromFile("../assets/cursor.png")) return -1;
    sf::Sprite cursor;
    cursor.setTexture(cursorSheet);
    cursor.setTextureRect(sf::IntRect(144, 0, 72, 72));
    int x_offset = -9;
    int y_offset = -4;

    // AUDIO
    sf::SoundBuffer sfxBuffer;
    if (!sfxBuffer.loadFromFile("../assets/foom.wav")) return -1;

    sf::Sound sfx;
    sfx.setBuffer(sfxBuffer);

    sf::Music music;
    if (!music.openFromFile(musicfiles.front())) return -1;
    music.setLoop(true);
    music.play();
    music.setVolume(5.f);

    sf::Clock clock; // starts the clock
    float dtime = 0;

    // GAMELOOP
    while (window.isOpen()) {
        sf::Event event;

        dtime = clock.restart().asSeconds(); // Get elapsed time
        player.update(dtime);                // Update Game
        view.setCenter(player.m_body.getPosition());

	/// Event loop /////////////////////////////////////////////////////////
        while (window.pollEvent(event)) {
            if (show_debug) ImGui::SFML::ProcessEvent(event);

            // TODO use switch
	    switch (event.type) {
	    case (sf::Event::Closed): window.close(); break;

	    case (sf::Event::Resized): resizeView(window, view); break;

            case (sf::Event::MouseButtonPressed):
	        cursor.setTextureRect(sf::IntRect(72, 0, 72, 72));
		break;

            case (sf::Event::MouseButtonReleased):
	        cursor.setTextureRect(sf::IntRect(144, 0, 72, 72));
		break;

            case (sf::Event::MouseWheelScrolled): {
                // ZOOMING
                int scrolltick = event.mouseWheelScroll.delta;
                if (scrolltick == 1) view.zoom(0.9f);
                if (scrolltick == -1) view.zoom(1.1f);
		break;
	    }

	    default: break;
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                // lerp player to mousepos (wip)
                player.m_body.setPosition(player.m_body.getPosition() * (1.0f - dtime) +
                                          window.mapPixelToCoords(sf::Mouse::getPosition(window)) * dtime);
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::F1))
                show_debug = !show_debug;
        }
	////////////////////////////////////////////////////////////////////////

	/// ImGUI //////////////////////////////////////////////////////////////
        if (show_debug) {
            // ImGui
            ImGui::SFML::Update(window, sf::seconds(dtime));
            ImGui::Begin("F1 to hide debug menu");
            ImGui::SliderInt("Cursor X-Offset", &x_offset, -10, 10);
            ImGui::SliderInt("Cursor Y-Offset", &y_offset, -10, 10);

            ImGui::Text("Choose song");
            ImGui::SameLine();
            if (ImGui::Button("Reload")) findAssets(exts, musicfiles);
            ImGui::SameLine();
            if (ImGui::Button("Pause")) music.pause();
            for (auto& file : musicfiles)
                if (ImGui::Button(file.c_str())) {
                    music.openFromFile(file);
                    music.play();
                }
            ImGui::End();
        }
	////////////////////////////////////////////////////////////////////////

	// collision
	if (player.m_body.getGlobalBounds().intersects(box.getGlobalBounds())) {
	    player.m_body.move(-player.movement);
	}

        // TODO use cursor class (?)
        // convert mousepos to world coordinates
        cursor.setPosition(window.mapPixelToCoords(sf::Vector2i(sf::Mouse::getPosition(window).x + x_offset,
                                                                sf::Mouse::getPosition(window).y + y_offset)));

        /// DRAW ///////////////////////////////////////////////////////////////
        window.clear(sf::Color(100, 180, 120, 255));
        window.setView(view);

        player.draw(window);
	window.draw(box);
        window.draw(text);

        if (show_debug) ImGui::SFML::Render(window);
        window.draw(cursor);
        window.display();
        ////////////////////////////////////////////////////////////////////////
    }

    return 0;
}

void findAssets(std::vector<std::string> exts,
                std::vector<std::string>& outfiles)
{
    bool match = false;
    outfiles.clear();
    std::string path("../assets/");

    for (auto& p : std::filesystem::recursive_directory_iterator(path)) {
        for (auto& ext : exts)
            if (p.path().extension() == ext) match = true;

        if (match) outfiles.push_back(p.path());
        match = false;
    }

    return;
}

void resizeView(const sf::RenderWindow& window, sf::View& view)
{
    float aspectratio_x = float(window.getSize().x) / float(window.getSize().y);
    float aspectratio_y = float(window.getSize().y) / float(window.getSize().x);
    view.setSize(VIEW_WIDTH * aspectratio_x, VIEW_HEIGHT * aspectratio_y);
}
