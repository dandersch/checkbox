#pragma once

#include "animation.h"
#include "entity.h"
#include "pch.h"

// forward declarations
template<typename Resource>
class ResourcePool;

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
    virtual void updateCurrent(float dt) override;
    void createAnimations();

    void handleEvent(const sf::Event& event, std::queue<Command>& commands);
    void handleInput(std::queue<Command>& commands);

    virtual unsigned int getCategory() const override;

    void assignKey(sf::Keyboard::Key key, Action action);
    sf::Keyboard::Key getAssignedKey(Action action) const;

    virtual sf::FloatRect getBoundingRect() const override
    {
        return getWorldTransform().transformRect(body.getGlobalBounds());
    };

private:
    void restartAnimsExcept(int index);
    void drawCurrent(sf::RenderTarget& target,
                     sf::RenderStates states) const override;
    static bool isOneShot(Action action);

public:
    bool canJump = false;
    bool running = false;
    bool facingRight = true;
    float speed;
    PlayerState m_state;
    sf::Sprite body;

private:
    //std::vector<Animation> m_anims;
    std::map<PlayerState, Animation> m_anims;
    std::map<sf::Keyboard::Key, Action> m_keybinds;
    std::map<Action, Command> m_actionbinds;
};
