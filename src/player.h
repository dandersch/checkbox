#pragma once
#include "event.h"
#include "pch.h"
#include "animation.h"
#include "entity.h"
#include "command.h"

#define PLAYER_WIDTH  128
#define PLAYER_HEIGHT 128

// forward declarations
template<typename Resource> class ResourcePool;
class Coin;
class Tile;

struct CheckboxInfo
{
    Entity* box;
    sf::Vector2f origPos;
};

class Player : public Entity
{
public:
    enum Action
    {
        MOVE_LEFT,
        MOVE_RIGHT,
        MOVE_UP,
        MOVE_DOWN,
        //SPRINT,
        JUMP,
        HOLD,
        DYING,
        RESPAWN,
        RETRY,
        WINNING,
        ACTIONCOUNT
    };

    enum PlayerState
    {
        IDLE,
        WALKING,
        RUNNING,
        JUMPING,
        FALLING,
        DEAD,
        CELEBRATING,
        STATECOUNT
    };

public:
    Player(ResourcePool<sf::Texture>& textures);
    ~Player()
    {
        EventSystem::unsubscribe(std::bind(&Player::processEvent, this,
                                           std::placeholders::_1));
    }

    virtual void updateCurrent(f32 dt) override;
    void createAnimations();

    void handleEvent(const sf::Event& event, std::queue<Command>& commands);
    void handleInput(std::queue<Command>& commands);

    inline u32 getType() const override { return ENTITY_PLAYER; }

    void assignKey(sf::Keyboard::Key key, Action action);
    sf::Keyboard::Key getAssignedKey(Action action) const;
    u32 getAssignedButton(Action action) const;
    void assignButton(u32 button, Action action);

    void processEvent(const Event& evn);

    virtual sf::FloatRect getBoundingRect() const override
    {
        return getWorldTransform().transformRect(m_sprite.getGlobalBounds());
    };

    inline sf::Vector2f forwardRay()
    {
        return facingRight ? sf::Vector2f(1, 0) : sf::Vector2f(-1, 0);
    }

    void retry();

private:
    void restartAnimsExcept(i32 index);
    void drawCurrent(sf::RenderTarget& target,
                     sf::RenderStates states) const override;
    static b32 isOneShot(Action action);

public:
    sf::Vector2f velocity = sf::Vector2f(0, 0);
    b32 canJump = false;
    b32 facingRight = true;
    b32 dead = false;
    b32 fixedJump = false;
    u32 lifeCount = 3;
    PlayerState m_state;
    sf::Vector2i spawn_loc;
    Entity* checkpoint_box = nullptr;
    bool gameWon = false;
    bool gameOver = false;
    sf::Clock celebTimer;
    Entity* holding = nullptr;
    std::function<bool(const Entity* lhs, const Entity* rhs)> set_comparator;
    std::set<Entity*, decltype(set_comparator)> holdables;
    std::vector<CheckboxInfo> checkboxes;
    std::vector<Coin*> collectedCoins;
    std::vector<Tile*> lyingCorpses;
    u32 goldCount = 0;
    sf::Sprite m_sprite;

    b32 leaveCorpse = false;
    b32 leaveDyingCorpse = false;
    sf::Vector2f deathPos;

private:
    std::map<PlayerState, Animation> m_anims;
    std::map<sf::Keyboard::Key, Action> m_keybinds;
    std::map<u32, Action> m_joybinds;
    std::map<Action, Command> m_actionbinds;
};
