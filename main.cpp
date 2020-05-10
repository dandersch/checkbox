#include "src/helper.h"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <glm/glm.hpp>

#include "imgui.h"
#include "imgui-SFML.h"

#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML works!");
    window.setFramerateLimit(60);
    //window.setVerticalSyncEnabled(true);
    ImGui::SFML::Init(window);
    window.setMouseCursorVisible(false);

    // TESTSHAPE
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Blue);
    shape.setPosition(500,500);


    // TEXTURE (8x9)
    int xstep = 64;
    int ystep = 64;
    sf::Texture texture;
    if (!texture.loadFromFile("../assets/platformer_sprites_base.png"))
        return -1;
    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect(0 * xstep, 8 * ystep, xstep, ystep));

    sf::Texture cursorSheet;
    if (!cursorSheet.loadFromFile("../assets/cursor.png")) return -1;

    sf::Sprite cursor;
    cursor.setTexture(cursorSheet);
    cursor.setTextureRect(sf::IntRect(144, 0 , 72, 72));


    // MOVEMENT
    //sf::Vector2f rightstep(3.0f, 0);
    //sf::Vector2f leftstep(-3.0f, 0);
    double speed = 0.f;
    float speedFactor = 1.f;


    // AUDIO
    sf::SoundBuffer sfxBuffer;
    if (!sfxBuffer.loadFromFile("../assets/foom.wav")) return -1;

    sf::Sound sfx;
    sfx.setBuffer(sfxBuffer);

    sf::Music music;
    if (!music.openFromFile("../assets/intro.ogg")) return -1;
    music.setLoop(true);
    music.play();


    sf::Clock clock; // starts the clock

    // GAMELOOP
    while (window.isOpen()) {
        sf::Event event;

	speed = 0.f;

        while (window.pollEvent(event)) {
	    ImGui::SFML::ProcessEvent(event);
	    // sf::Time elapsed = clock.restart();
	    // updateGame(elapsed);
            if (event.type == sf::Event::Closed)
                window.close();

	    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
	        sprite.setTextureRect(sf::IntRect(7 * xstep, 0 * ystep,
						  -xstep, ystep));
		speed = -100.0f * speedFactor;
		sfx.play();
	    }

	    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
	        sprite.setTextureRect(sf::IntRect(5 * xstep, 7 * ystep,
						  xstep, ystep));
		speed = 100.0f * speedFactor;
		sfx.play();
	    }

            if (event.type == sf::Event::MouseMoved)
            {
	        float x_offset = 6;
	        float y_offset = 2;
	        cursor.setPosition(event.mouseMove.x - x_offset,
				   event.mouseMove.y - y_offset);
            }

            if (event.type == sf::Event::MouseButtonPressed)
                cursor.setTextureRect(sf::IntRect(72, 0 , 72, 72));

            if (event.type == sf::Event::MouseButtonReleased)
                cursor.setTextureRect(sf::IntRect(144, 0 , 72, 72));
        }

        // Get elapsed time
	sf::Time delta = clock.restart();

	// Update sprite position
        sprite.move(speed * delta.asSeconds(), 0.f );

	// ImGui
        ImGui::SFML::Update(window, delta);
        ImGui::Begin("Hello, world!");
        ImGui::SliderFloat("Set speed", &speedFactor, 0.1f, 10.f);
        ImGui::End();

        window.clear(sf::Color(100,180,120,255));

	// DRAW
        window.draw(shape);
	window.draw(sprite);
	window.draw(cursor);

	ImGui::SFML::Render(window);
        window.display();
    }

    return 0;
}
