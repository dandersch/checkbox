#include "entity.h"
#include "resourcepool.h"


std::string toTextureFile(Enemy::Type type)
{
    switch (type) {
    case Enemy::Slave: return "enemy_slave.png";
    case Enemy::Skeleton: return "enemy_skeleton.png";
    }
}

Enemy::Enemy(Type type, ResourcePool<sf::Texture>& textures)
  : m_type(type)
  , m_sprite(textures.get(toTextureFile(type)))
{
    sf::IntRect frame;

    switch (type) {
    case Enemy::Slave:
        frame.left = 0;
        frame.top = 0;
        frame.width = 64;
        frame.height = 64;
        break;

    case Enemy::Skeleton:
        frame.left = 0;
        frame.top = 0;
        frame.width = 36;
        frame.height = 48;
        break;
    }

    m_sprite.setTextureRect(frame);
    m_sprite.setOrigin(m_sprite.getLocalBounds().width / 2.f,
                       m_sprite.getLocalBounds().height / 2.f);
}

void Enemy::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_sprite, states);
}
