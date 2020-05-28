#pragma once

#include "pch.h"
#include "animation.h"
#include "entity.h"
#include "command.h"

// forward declarations
template<typename Resource> class ResourcePool;

class Player : public Entity
{
public:
    enum Action
    {
        MOVE_LEFT,
        MOVE_RIGHT,
        MOVE_UP,
        MOVE_DOWN,
        SPRINT,
        JUMP,
        ACTIONCOUNT
    };

    enum PlayerState
    {
        IDLE,
        WALKING,
        RUNNING,
        JUMPING,
        FALLING,
        //DEAD,
        STATECOUNT
    };

public:
    Player(ResourcePool<sf::Texture>& textures);
    virtual void updateCurrent(f32 dt) override;
    void createAnimations();

    void handleEvent(const sf::Event& event, std::queue<Command>& commands);
    void handleInput(std::queue<Command>& commands);

    inline u32 getCategory() const override { return Category::Player; }

    void assignKey(sf::Keyboard::Key key, Action action);
    sf::Keyboard::Key getAssignedKey(Action action) const;

    virtual sf::FloatRect getBoundingRect() const override
    {
        return getWorldTransform().transformRect(m_sprite.getGlobalBounds());
    };

private:
    void restartAnimsExcept(i32 index);
    void drawCurrent(sf::RenderTarget& target,
                     sf::RenderStates states) const override;
    static b32 isOneShot(Action action);

public:
    sf::Vector2f velocity = sf::Vector2f(0, 0);
    b32 canJump = false;
    b32 running = false;
    b32 facingRight = true;
    f32 speed;
    PlayerState m_state;

private:
    std::map<PlayerState, Animation> m_anims;
    std::map<sf::Keyboard::Key, Action> m_keybinds;
    std::map<Action, Command> m_actionbinds;
    sf::Sprite m_sprite;
};
