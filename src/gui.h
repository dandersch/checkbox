#pragma once
#include "pch.h"
#include "resourcepool.h"

class GUIComponent
  : public sf::Drawable
  , public sf::Transformable
  , private sf::NonCopyable
{
public:
    GUIComponent() {}
    virtual ~GUIComponent() {}
    virtual bool isSelectable() const = 0;
    virtual void handleEvent(const sf::Event& event) = 0;
    inline bool isSelected() const { return mIsSelected; }
    inline virtual void select() { mIsSelected = true; }
    inline virtual void deselect() { mIsSelected = false; }
    inline virtual bool isActive() const { return mIsActive; }
    inline virtual void activate() { mIsActive = true; }
    inline virtual void deactivate() { mIsActive = false; }

private:
    bool mIsSelected = false;
    bool mIsActive   = false;
};

class GUIContainer : public GUIComponent
{
public:
    GUIContainer() : mChildren() , mSelectedChild(-1) {}

    void pack(std::shared_ptr<GUIComponent> component)
    {
        mChildren.push_back(component);
        if (!hasSelection() && component->isSelectable())
            select(mChildren.size() - 1);
    }

    inline virtual bool isSelectable() const { return false; }

    virtual void handleEvent(const sf::Event& event)
    {
        if (hasSelection() && mChildren[mSelectedChild]->isActive())
        {
            mChildren[mSelectedChild]->handleEvent(event);
        }
        else if (event.type == sf::Event::KeyReleased)
        {
            if (event.key.code == sf::Keyboard::Up)
            {
                selectPrevious();
            }
            else if (event.key.code == sf::Keyboard::Down)
            {
                selectNext();
            }
            else if (event.key.code == sf::Keyboard::Return)
            {
                if (hasSelection()) mChildren[mSelectedChild]->activate();
            }
        }
    }

    void select(std::size_t index)
    {
        if (mChildren[index]->isSelectable())
        {
            if (hasSelection()) mChildren[mSelectedChild]->deselect();
            mChildren[index]->select();
            mSelectedChild = index;
        }
    }

    void selectNext()
    {
        if (!hasSelection()) return;

        // Search next component that is selectable
        int next = mSelectedChild;
        do
            next = (next + 1) % mChildren.size();
        while (!mChildren[next]->isSelectable());

        // Select that component
        select(next);
    }

    void selectPrevious()
    {
        if (!hasSelection()) return;

        // Search previous component that is selectable, wrap around if
        // necessary
        int prev = mSelectedChild;
        do
            prev = (prev + mChildren.size() - 1) % mChildren.size();
        while (!mChildren[prev]->isSelectable());

        // Select that component
        select(prev);
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();

        for (const std::shared_ptr<GUIComponent>& child : mChildren)
            target.draw(*child, states);
    }

    inline bool hasSelection() const { return mSelectedChild >= 0; }

private:
    std::vector<std::shared_ptr<GUIComponent>> mChildren;
    i32 mSelectedChild;
};

class GUILabel : public GUIComponent
{
public:
    GUILabel(const std::string& text, ResourcePool<sf::Font>& fonts)
      : mText(text, fonts.get("Boxy-Bold.ttf"), 100)
    {
    }

    virtual bool isSelectable() const { return false; }
    inline void setText(const std::string& text) { mText.setString(text); }
    inline void setColor(const sf::Color col) { mText.setFillColor(col); }
    virtual void handleEvent(const sf::Event& event) {}

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();
        target.draw(mText, states);
    }

private:
    sf::Text mText;
};

class GUIButton : public GUIComponent
{
public:
    GUIButton(ResourcePool<sf::Font>& fonts,
              ResourcePool<sf::Texture>& textures)
      : mCallback()
      , mNormalTexture(textures.get("button1.png"))
      , mSelectedTexture(textures.get("button2.png"))
      , mPressedTexture(textures.get("button3.png"))
      , mSprite()
      , mText("", fonts.get("Boxy-Bold.ttf"), 32)
      , mIsToggle(false)
    {
        mSprite.setTexture(mNormalTexture);

        sf::FloatRect bounds = mSprite.getLocalBounds();
        mText.setPosition(bounds.width / 2.f, bounds.height / 2.f);
    }

    void setCallback(std::function<void()> callback)
    {
        mCallback = std::move(callback);
    }

    void setText(const std::string& text)
    {
        mText.setString(text);

        // center origin
        sf::FloatRect bounds = mText.getLocalBounds();
        mText.setOrigin(std::floor(bounds.left + bounds.width / 2.f),
                        std::floor(bounds.top + bounds.height / 2.f));
    }

    inline void setToggle(bool flag) { mIsToggle = flag; }
    inline virtual bool isSelectable() const { return true; }

    virtual void select()
    {
        GUIComponent::select();

        mSprite.setTexture(mSelectedTexture);
    }

    virtual void deselect()
    {
        GUIComponent::deselect();

        mSprite.setTexture(mNormalTexture);
    }

    virtual void activate()
    {
        GUIComponent::activate();

        // If we are toggle then we should show that the button is pressed and
        // thus "toggled".
        if (mIsToggle) mSprite.setTexture(mPressedTexture);

        if (mCallback) mCallback();

        // If we are not a toggle then deactivate the button since we are just
        // momentarily activated.
        if (!mIsToggle) deactivate();
    }

    virtual void deactivate()
    {
        GUIComponent::deactivate();

        if (mIsToggle)
        {
            // Reset texture to right one depending on if we are selected or
            // not.
            if (isSelected()) mSprite.setTexture(mSelectedTexture);
            else mSprite.setTexture(mNormalTexture);
        }
    }

    virtual void handleEvent(const sf::Event& event) {}

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();
        target.draw(mSprite, states);
        target.draw(mText, states);
    }

private:
    std::function<void()> mCallback;
    const sf::Texture& mNormalTexture;
    const sf::Texture& mSelectedTexture;
    const sf::Texture& mPressedTexture;
    sf::Sprite mSprite;
    sf::Text mText;
    bool mIsToggle;
};
