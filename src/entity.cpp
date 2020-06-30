#include "entity.h"
#include "command.h"

Entity::Entity()
  : m_children()
{
    m_children.reserve(5);
}

void Entity::attachChild(std::unique_ptr<Entity> child)
{
    child->m_parent = this;
    m_children.push_back(std::move(child));
}

std::unique_ptr<Entity> Entity::detachChild(const Entity& node)
{
    auto found = std::find_if(m_children.begin(), m_children.end(),
                              [&](std::unique_ptr<Entity>& p) -> b32 {
                                  return p.get() == &node;
                              });

    assert(found != m_children.end());
    std::unique_ptr<Entity> result = std::move(*found);
    result->m_parent = nullptr;
    m_children.erase(found);

    return result;
}

void Entity::update(f32 dt)
{
    updateCurrent(dt);
    updateChildren(dt);
}

// TODO(dan): why does this break when removed
sf::FloatRect Entity::getBoundingRect() const
{
    sf::FloatRect test;
    return test;
}

void Entity::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    drawCurrent(target, states);

    // draw children
    for (const auto& child : m_children) child->draw(target, states);
}

void Entity::updateChildren(f32 dt)
{
    for (auto& child : m_children) child->update(dt);
}

// TODO crashes
sf::Transform Entity::getWorldTransform() const
{
    sf::Transform xform = sf::Transform::Identity;

    // traverse the tree upwards
    for (const Entity* node = this; node != nullptr; node = node->m_parent)
        xform = node->getTransform() * xform;

    return xform;
}

u32 Entity::getType() const
{
    return ENTITY_NONE;
}

// TODO(dan): improve performance by getting tiles out of the scenegraph
void Entity::onCommand(const Command& command, f32 dt)
{
    if (command.category & getType()) command.action(*this, dt);

    for (std::unique_ptr<Entity>& child : m_children)
        child->onCommand(command, dt);
}
