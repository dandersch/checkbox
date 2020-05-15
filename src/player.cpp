#include "pch.h"

#include "player.h"

/*
 *  TODO: show IDLE_LEFT after WALK_LEFT
 */

Player::Player() {}
Player::~Player() {}

/*
 * TODO: set/unset booleans w/ KeyPressed/KeyReleased ?
 * TODO: don't read keyinputs if window not focused
 */
void Player::update(float dtime)
{
    movement = sf::Vector2f(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        movement.x = -speed * 1.0f;
        m_state = WALKING_LEFT;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        movement.x = speed * 1.0f;
        m_state = WALKING_RIGHT;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) {
        m_state = DEAD;
    } else {
        m_state = IDLE;
    }

    // TODO modifier supposed to be used like this?
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) &&
        sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        movement.x = speed * 2.5f;
        m_state = RUN_RIGHT;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) &&
               sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        movement.x = -speed * 2.5f;
        m_state = RUN_LEFT;
    }

    restartAnimsExcept(m_state);

    body.move(movement);
    body.setTextureRect(m_anims[m_state].update(dtime));
}

void Player::setTexture(const sf::Texture& texture)
{
    body.setTexture(texture);
    m_anims.clear();

    // TODO crash w/o anims
    /* Idle */
    m_anims.emplace_back(texture.getSize(), 8, 9);
    m_anims.back().add(0, 2, 0.4f);
    m_anims.back().add(1, 2, 0.4f);

    /* Walk right */
    m_anims.emplace_back(texture.getSize(), 8, 9);
    m_anims.back().add(0, 4, 0.1f);
    m_anims.back().add(1, 4, 0.1f);
    m_anims.back().add(2, 4, 0.1f);
    m_anims.back().add(3, 4, 0.1f);
    m_anims.back().add(4, 4, 0.1f);
    m_anims.back().add(5, 4, 0.1f);
    m_anims.back().add(6, 4, 0.1f);
    m_anims.back().add(7, 4, 0.1f);

    /* Walk left */
    m_anims.emplace_back(texture.getSize(), 8, 9);
    m_anims.back().add(0, 4, 0.1f, true);
    m_anims.back().add(1, 4, 0.1f, true);
    m_anims.back().add(2, 4, 0.1f, true);
    m_anims.back().add(3, 4, 0.1f, true);
    m_anims.back().add(4, 4, 0.1f, true);
    m_anims.back().add(5, 4, 0.1f, true);
    m_anims.back().add(6, 4, 0.1f, true);
    m_anims.back().add(7, 4, 0.1f, true);

    /* Death */
    m_anims.emplace_back(texture.getSize(), 8, 9);
    m_anims.back().add(0, 2, 0.2f);
    m_anims.back().add(1, 2, 0.2f);
    m_anims.back().add(2, 2, 0.2f);
    m_anims.back().add(3, 2, 0.2f);
    m_anims.back().add(4, 2, 0.2f);
    m_anims.back().add(5, 2, 0.2f);
    m_anims.back().add(6, 2, 0.2f);
    m_anims.back().add(7, 2, 0.2f);
    m_anims.back().looped = false;

    /* Run right */
    m_anims.emplace_back(texture.getSize(), 8, 9);
    m_anims.back().add(4, 0, 0.1f);
    m_anims.back().add(5, 0, 0.1f);
    m_anims.back().add(6, 0, 0.1f);
    m_anims.back().add(7, 0, 0.1f);
    m_anims.back().add(0, 1, 0.1f);
    m_anims.back().add(1, 1, 0.1f);
    m_anims.back().add(2, 1, 0.1f);
    m_anims.back().add(3, 1, 0.1f);

    /* Run left */
    m_anims.emplace_back(texture.getSize(), 8, 9);
    m_anims.back().add(4, 0, 0.1f, true);
    m_anims.back().add(5, 0, 0.1f, true);
    m_anims.back().add(6, 0, 0.1f, true);
    m_anims.back().add(7, 0, 0.1f, true);
    m_anims.back().add(0, 1, 0.1f, true);
    m_anims.back().add(1, 1, 0.1f, true);
    m_anims.back().add(2, 1, 0.1f, true);
    m_anims.back().add(3, 1, 0.1f, true);
}

void Player::draw(sf::RenderWindow& window)
{
    window.draw(body);
}

void Player::restartAnimsExcept(int index)
{
    for (int i = 0; i < m_anims.size(); i++) {
        // check if needs restart (?)
        if (i != index) m_anims[i].restart();
    }
}
