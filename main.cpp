#include "src/helper.h"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <glm/glm.hpp>

#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML works!");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    // TEXTURE (8x9)
    int xstep = 64;
    int ystep = 64;
    sf::Texture texture;
    if (!texture.loadFromFile("../assets/platformer_sprites_base.png"))
        return -1;
    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect(0 * xstep, 8 * ystep, xstep, ystep));

    // MOVEMENT
    sf::Vector2f rightstep(3.0f, 0);
    sf::Vector2f leftstep(-3.0f, 0);

    // AUDIO
    sf::SoundBuffer sfxBuffer;
    if (!sfxBuffer.loadFromFile("../assets/foom.wav")) return -1;

    sf::Sound sfx;
    sfx.setBuffer(sfxBuffer);

    sf::Music music;
    if (!music.openFromFile("../assets/intro.ogg")) return -1;
    music.setLoop(true);
    music.play();

    // GAMELOOP
    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

	    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
	        sprite.setTextureRect(sf::IntRect(7 * xstep, 0 * ystep,
						  -xstep, ystep));
		sprite.setPosition(sprite.getPosition() + leftstep);
		sfx.play();
	    }

	    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
	        sprite.setTextureRect(sf::IntRect(5 * xstep, 7 * ystep,
						  xstep, ystep));
		sprite.setPosition(sprite.getPosition() + rightstep);
		sfx.play();
	    }
        }

        window.clear();

	// DRAW
        window.draw(shape);
	window.draw(sprite);

        window.display();
    }

    return 0;
}
