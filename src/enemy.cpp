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
{}

void Enemy::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_sprite, states);
}
