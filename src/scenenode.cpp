#include "scenenode.h"
#include "command.h"

SceneNode::SceneNode()
  : m_children()
  , m_parent(nullptr)
{}

void SceneNode::attachChild(std::unique_ptr<SceneNode> child)
{
    child->m_parent = this;
    m_children.push_back(std::move(child));
}

std::unique_ptr<SceneNode> SceneNode::detachChild(const SceneNode& node)
{
    auto found = std::find_if(m_children.begin(), m_children.end(),
                              [&](std::unique_ptr<SceneNode>& p) -> bool {
                                  return p.get() == &node;
                              });

    assert(found != m_children.end());
    std::unique_ptr<SceneNode> result = std::move(*found);
    result->m_parent = nullptr;
    m_children.erase(found);

    return result;
}

void SceneNode::update(float dt)
{
    updateCurrent(dt);
    updateChildren(dt);
}

sf::FloatRect SceneNode::getBoundingRect() const
{
    sf::FloatRect test;
    return test;
}

void SceneNode::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    drawCurrent(target, states);

    // draw children
    for (const auto& child : m_children) child->draw(target, states);
}

void SceneNode::updateChildren(float dt)
{
    for (auto& child : m_children) child->update(dt);
}

// TODO crashes
sf::Transform SceneNode::getWorldTransform() const
{
    sf::Transform xform = sf::Transform::Identity;

    // traverse the tree upwards
    for (const SceneNode* node = this; node != nullptr; node = node->m_parent)
        xform = node->getTransform() * xform;

    return xform;
}

unsigned int SceneNode::getCategory() const
{
    return Category::Scene;
}

void SceneNode::onCommand(const Command& command, float dt)
{
    if (command.category & getCategory()) command.action(*this, dt);

    for (std::unique_ptr<SceneNode>& child : m_children)
        child->onCommand(command, dt);
}

void SceneNode::checkCollisions(sf::FloatRect& collider,
                                collisionInfo& movement)
{
    sf::FloatRect collision;

    if (getCategory() != Category::Player && this->m_parent != nullptr &&
        collider.intersects(getBoundingRect(), collision)) {
        movement.movement = collision;
        movement.halfsize = sf::Vector2f(getBoundingRect().width / 2,
                                         getBoundingRect().height / 2);
        movement.position = getPosition();
        movement.collided = true;

    } else {
        for (const auto& child : m_children)
            child->checkCollisions(collider, movement);
    }
}
