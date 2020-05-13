#include "pch.h"

#include "player.h"

Player::Player(sf::Texture* texture, float speed)
  : m_tex(texture), m_speed(speed)
{
    m_body.setOrigin(32, 32);
    m_body.setPosition(750, 500);
    m_body.setTexture(*texture);

    m_anims.emplace_back(m_tex, 8, 9);
    m_anims[0].add(0, 2, 0.2f);
    m_anims[0].add(1, 2, 0.2f);
    m_anims[0].add(2, 2, 0.2f);
    m_anims[0].add(3, 2, 0.2f);
    m_anims[0].add(4, 2, 0.2f);
    m_anims[0].add(5, 2, 0.2f);
    m_anims[0].add(6, 2, 0.2f);
    m_anims[0].add(7, 2, 0.2f);
    m_anims[0].add(7, 2, 0.2f, true);
    m_anims[0].add(6, 2, 0.2f, true);
    m_anims[0].add(5, 2, 0.2f, true);
    m_anims[0].add(4, 2, 0.2f, true);
    m_anims[0].add(3, 2, 0.2f, true);
    m_anims[0].add(2, 2, 0.2f, true);
    m_anims[0].add(1, 2, 0.2f, true);

    /*
    Animation walkright(&playerTexture, 8, 9);
    walkright.add(0, 4, 0.1f);
    walkright.add(1, 4, 0.1f);
    walkright.add(2, 4, 0.1f);
    walkright.add(3, 4, 0.1f);
    walkright.add(4, 4, 0.1f);
    walkright.add(5, 4, 0.1f);
    walkright.add(6, 4, 0.1f);
    walkright.add(7, 4, 0.1f);

    Animation walkleft(&playerTexture, 8, 9);
    walkleft.add(0, 4, 0.1f, true);
    walkleft.add(1, 4, 0.1f, true);
    walkleft.add(2, 4, 0.1f, true);
    walkleft.add(3, 4, 0.1f, true);
    walkleft.add(4, 4, 0.1f, true);
    walkleft.add(5, 4, 0.1f, true);
    walkleft.add(6, 4, 0.1f, true);
    walkleft.add(7, 4, 0.1f, true);
    */

}

Player::~Player() {}

void Player::update(float dtime)
{
    sf::Vector2f movement(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        movement.x = -m_speed  * 1.0f;
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        movement.x =  m_speed  * 1.0f;
    else
        ;

    m_body.move(movement);
    m_body.setTextureRect(m_anims[0].update(dtime));
}

void Player::draw(sf::RenderWindow& window)
{
    window.draw(m_body);
}
