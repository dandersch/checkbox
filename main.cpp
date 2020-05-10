#include "src/helper.h"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <glm/glm.hpp>

#include <iostream>

int main()
{
    glm::vec3 testvec(1.0f,2.0f,3.0f);

    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    // AUDIO
    sf::SoundBuffer sfxBuffer;
    if (!sfxBuffer.loadFromFile("../assets/foom.wav")) return -1;

    sf::Sound sfx;
    sfx.setBuffer(sfxBuffer);

    sf::Music music;
    if (!music.openFromFile("../assets/intro.ogg")) return -1;
    music.play();

    // GAMELOOP
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

	    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		sfx.play();
        }

        window.clear();
        window.draw(shape);
        window.display();
    }

    printf("%d", helperfunc());
    printf("%f", testvec[1]);

    return 0;
}
