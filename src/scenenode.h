#pragma once

#include "pch.h"

class SceneNode
  : public sf::Transformable
  , public sf::Drawable
  , private sf::NonCopyable
{
public:
    void attachChild(std::unique_ptr<SceneNode> child)
    {
        child->m_parent = this;
        m_children.push_back(std::move(child));
    }

    std::unique_ptr<SceneNode> detachChild(const SceneNode& node)
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

    void update(float dt)
    {
        updateCurrent(dt);
        updateChildren(dt);
    }

private:
    virtual void draw(sf::RenderTarget& target,
                      sf::RenderStates states) const override final
    {
        states.transform *= getTransform();

        drawCurrent(target, states);

        // draw children
        for (const auto& child : m_children)
            child->draw(target, states);
    }

    virtual void drawCurrent(sf::RenderTarget& target,
                             sf::RenderStates states) const {}

    virtual void updateCurrent(float dt) {}

    void updateChildren(float dt)
    {
        for (auto& child : m_children)
            child->update(dt);
    }

private:
    std::vector<std::unique_ptr<SceneNode>> m_children;
    SceneNode* m_parent;
};
