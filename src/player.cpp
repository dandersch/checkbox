#include "player.h"
#include "resourcepool.h"
#include "command.h"

// functor for player movement
struct PlayerMover
{
    PlayerMover(float vx, float vy) : velocity(vx, vy) {}
    void operator()(Player& player, float) const
    {
        player.velocity += velocity;
    }

    sf::Vector2f velocity;
};


/*
 *  TODO: show IDLE_LEFT after WALK_LEFT
 */
Player::Player(ResourcePool<sf::Texture>& textures)
  : body(textures.get("platformer_sprites_base.png"))
  , speed(75.f)
{
    m_state = IDLE;
    createAnimations();

    assignKey(sf::Keyboard::A, MOVE_LEFT);
    assignKey(sf::Keyboard::D, MOVE_RIGHT);
    assignKey(sf::Keyboard::LShift, SPRINT);

    m_actionbinds[MOVE_LEFT].action  = derivedAction<Player>(PlayerMover(-speed, 0.f));
    m_actionbinds[MOVE_RIGHT].action = derivedAction<Player>(PlayerMover(+speed, 0.f));

    for (auto& i : m_actionbinds) i.second.category = Category::Player;
}

/*
 * TODO: set/unset booleans w/ KeyPressed/KeyReleased ?
 * TODO: don't read keyinputs if window not focused
 */
void Player::updateCurrent(float dt)
{
    restartAnimsExcept(m_state);
    move(velocity * dt);
    body.setTextureRect(m_anims[m_state].update(dt));
}

void Player::createAnimations()
{
    sf::Vector2u texSize = body.getTexture()->getSize();
    m_anims.clear();

    // TODO crash w/o anims
    /* Idle */
    m_anims.emplace_back(texSize, 8, 9);
    m_anims.back().add(0, 2, 0.4f);
    m_anims.back().add(1, 2, 0.4f);

    /* Walk right */
    m_anims.emplace_back(texSize, 8, 9);
    m_anims.back().add(0, 4, 0.1f);
    m_anims.back().add(1, 4, 0.1f);
    m_anims.back().add(2, 4, 0.1f);
    m_anims.back().add(3, 4, 0.1f);
    m_anims.back().add(4, 4, 0.1f);
    m_anims.back().add(5, 4, 0.1f);
    m_anims.back().add(6, 4, 0.1f);
    m_anims.back().add(7, 4, 0.1f);

    /* Walk left */
    m_anims.emplace_back(texSize, 8, 9);
    m_anims.back().add(0, 4, 0.1f, true);
    m_anims.back().add(1, 4, 0.1f, true);
    m_anims.back().add(2, 4, 0.1f, true);
    m_anims.back().add(3, 4, 0.1f, true);
    m_anims.back().add(4, 4, 0.1f, true);
    m_anims.back().add(5, 4, 0.1f, true);
    m_anims.back().add(6, 4, 0.1f, true);
    m_anims.back().add(7, 4, 0.1f, true);

    /* Death */
    m_anims.emplace_back(texSize, 8, 9);
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
    m_anims.emplace_back(texSize, 8, 9);
    m_anims.back().add(4, 0, 0.1f);
    m_anims.back().add(5, 0, 0.1f);
    m_anims.back().add(6, 0, 0.1f);
    m_anims.back().add(7, 0, 0.1f);
    m_anims.back().add(0, 1, 0.1f);
    m_anims.back().add(1, 1, 0.1f);
    m_anims.back().add(2, 1, 0.1f);
    m_anims.back().add(3, 1, 0.1f);

    /* Run left */
    m_anims.emplace_back(texSize, 8, 9);
    m_anims.back().add(4, 0, 0.1f, true);
    m_anims.back().add(5, 0, 0.1f, true);
    m_anims.back().add(6, 0, 0.1f, true);
    m_anims.back().add(7, 0, 0.1f, true);
    m_anims.back().add(0, 1, 0.1f, true);
    m_anims.back().add(1, 1, 0.1f, true);
    m_anims.back().add(2, 1, 0.1f, true);
    m_anims.back().add(3, 1, 0.1f, true);
}

void Player::restartAnimsExcept(int index)
{
    for (int i = 0; i < m_anims.size(); i++) {
        // check if needs restart (?)
        if (i != index) m_anims[i].restart();
    }
}

void Player::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(body, states);
}

// For one-time actions (WHEN an event happens)
void Player::handleEvent(const sf::Event& event, std::queue<Command>& commands)
{
    // workaround to support modifiers
    switch (event.type) {
    case sf::Event::KeyPressed:
        if (event.key.code == getAssignedKey(SPRINT)) {
            m_actionbinds[MOVE_LEFT].action  = derivedAction<Player>(PlayerMover(-speed * 2, 0.f));
            m_actionbinds[MOVE_RIGHT].action = derivedAction<Player>(PlayerMover(+speed * 2, 0.f));
            for (auto& i : m_actionbinds) i.second.category = Category::Player;
        }
        break;
    case sf::Event::KeyReleased:
        if (event.key.code == getAssignedKey(SPRINT)) {
            m_actionbinds[MOVE_LEFT].action  = derivedAction<Player>(PlayerMover(-speed, 0.f));
            m_actionbinds[MOVE_RIGHT].action = derivedAction<Player>(PlayerMover(+speed, 0.f));
            for (auto& i : m_actionbinds) i.second.category = Category::Player;
        }
        break;
    default: break;
    }
}

// For continuous real-time actions (WHILE an event happens)
void Player::handleInput(std::queue<Command>& commands)
{
    m_state = IDLE;

    /*
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) &&
        sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
        m_state = RUNNING_RIGHT;
        Command sprintRight;
        sprintRight.category = Category::Player;
        sprintRight.action = derivedAction<Player>(PlayerMover(speed * 2,
                                                               0.0f));
        commands.push(sprintRight);
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        m_state = WALKING_RIGHT;
        Command walkRight;
        walkRight.category = Category::Player;
        walkRight.action = derivedAction<Player>(PlayerMover(speed, 0.0f));
        commands.push(walkRight);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) &&
        sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
        m_state = RUNNING_LEFT;
        Command sprintLeft;
        sprintLeft.category = Category::Player;
        sprintLeft.action = derivedAction<Player>(PlayerMover(-speed * 2,
                                                              0.0f));
        commands.push(sprintLeft);
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        m_state = WALKING_LEFT;
        Command walkLeft;
        walkLeft.category = Category::Player;
        walkLeft.action = derivedAction<Player>(PlayerMover(-speed, 0.0f));
        commands.push(walkLeft);
    }
    */

    for (auto i : m_keybinds)
        if (sf::Keyboard::isKeyPressed(i.first) && !isOneShot(i.second))
            commands.push(m_actionbinds[i.second]);
}

unsigned int Player::getCategory() const
{
    return Category::Player;
}

bool Player::isOneShot(Action action)
{
    switch (action) {
    case MOVE_LEFT:  // fallthrough
    case MOVE_RIGHT: return false;

    default: return true;
    }
}

void Player::assignKey(sf::Keyboard::Key key, Action action)
{
    // Remove all keys that already map to action
    for (auto it = m_keybinds.begin(); it != m_keybinds.end();) {
        if (it->second == action) m_keybinds.erase(it++);
        else ++it;
    }

    // Insert new binding
    m_keybinds[key] = action;
}

sf::Keyboard::Key Player::getAssignedKey(Action action) const
{
    for (auto i : m_keybinds)
        if (i.second == action) return i.first;

    return sf::Keyboard::Unknown;
}
