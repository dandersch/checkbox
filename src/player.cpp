#include "player.h"
#include "resourcepool.h"
#include "command.h"

// functor for player movement
struct PlayerMover
{
    PlayerMover(float vx, float vy, bool rightDir)
      : velocity(vx, vy)
      , rightDir(rightDir)
    {}
    void operator()(Player& player, float) const
    {
        if (player.running) {
            player.velocity += (2.f * velocity);
            if (player.canJump) player.m_state = Player::RUNNING;
        } else {
            player.velocity += velocity;
            if (player.canJump) player.m_state = Player::WALKING;
        }

        player.facingRight = rightDir;
    }

    sf::Vector2f velocity;
    bool rightDir;
};

/*
 *  TODO: show IDLE_LEFT after WALK_LEFT
 */
Player::Player(ResourcePool<sf::Texture>& textures)
  : body(textures.get("platformer_sprites_base.png"))
  , speed(75.f)
{
    m_state = IDLE;
    body.setTextureRect(sf::IntRect(0, 0, 64, 64));
    body.setOrigin(32,32);
    createAnimations();

    assignKey(sf::Keyboard::A, MOVE_LEFT);
    assignKey(sf::Keyboard::D, MOVE_RIGHT);
    assignKey(sf::Keyboard::W, MOVE_UP);
    assignKey(sf::Keyboard::S, MOVE_DOWN);
    assignKey(sf::Keyboard::LShift, SPRINT);
    assignKey(sf::Keyboard::Space, JUMP);

    m_actionbinds[MOVE_LEFT].action  = derivedAction<Player>(PlayerMover(-speed, 0.f, false));
    m_actionbinds[MOVE_RIGHT].action = derivedAction<Player>(PlayerMover(+speed, 0.f, true));
    //m_actionbinds[MOVE_UP].action  = derivedAction<Player>(PlayerMover(0.f, -speed));
    //m_actionbinds[MOVE_DOWN].action = derivedAction<Player>(PlayerMover(0.f, +speed * 10));
    m_actionbinds[SPRINT].action = derivedAction<Player>([](Player& p, float) {
        p.running = !p.running;
    });
    m_actionbinds[JUMP].action = derivedAction<Player>([](Player& p, float) {
        if (p.canJump) {
            p.velocity.y = -sqrtf(2.0f * 981.f * 120.f);
            p.m_state = JUMPING;
            p.canJump = false;
        }
    });

    for (auto& i : m_actionbinds) i.second.category = Category::Player;
}

/*
 * TODO: set/unset booleans w/ KeyPressed/KeyReleased ?
 * TODO: don't read keyinputs if window not focused
 */
void Player::updateCurrent(float dt)
{
    move(velocity * dt);

    if (velocity.y > 30.f) m_state = FALLING;

    // only update texture if animation was found
    if (m_anims.find(m_state) != m_anims.end()) {
        restartAnimsExcept(m_state);
        m_anims.at(m_state).flipped = !facingRight;
        body.setTextureRect(m_anims.at(m_state).update(dt));
    }
}

void Player::createAnimations()
{
    sf::Vector2u texSize = body.getTexture()->getSize();
    m_anims.clear();

    /* Idle */
    m_anims.emplace(IDLE, Animation(texSize, 8, 9));
    m_anims.at(IDLE).add(0, 2, 0.4f);
    m_anims.at(IDLE).add(1, 2, 0.4f);

    /* Walking */
    m_anims.emplace(WALKING, Animation(texSize, 8, 9));
    m_anims.at(WALKING).add(0, 4, 0.1f);
    m_anims.at(WALKING).add(1, 4, 0.1f);
    m_anims.at(WALKING).add(2, 4, 0.1f);
    m_anims.at(WALKING).add(3, 4, 0.1f);
    m_anims.at(WALKING).add(4, 4, 0.1f);
    m_anims.at(WALKING).add(5, 4, 0.1f);
    m_anims.at(WALKING).add(6, 4, 0.1f);
    m_anims.at(WALKING).add(7, 4, 0.1f);

    /* Running */
    m_anims.emplace(RUNNING, Animation(texSize, 8, 9));
    m_anims.at(RUNNING).add(4, 0, 0.1f);
    m_anims.at(RUNNING).add(5, 0, 0.1f);
    m_anims.at(RUNNING).add(6, 0, 0.1f);
    m_anims.at(RUNNING).add(7, 0, 0.1f);
    m_anims.at(RUNNING).add(0, 1, 0.1f);
    m_anims.at(RUNNING).add(1, 1, 0.1f);
    m_anims.at(RUNNING).add(2, 1, 0.1f);
    m_anims.at(RUNNING).add(3, 1, 0.1f);

    /* Jump */
    m_anims.emplace(JUMPING, Animation(texSize, 8, 9));
    m_anims.at(JUMPING).add(2, 5, 0.05f);
    m_anims.at(JUMPING).add(3, 5, 0.05f);
    m_anims.at(JUMPING).add(4, 5, 0.1f);
    m_anims.at(JUMPING).looped = false;

    m_anims.emplace(FALLING, Animation(texSize, 8, 9));
    m_anims.at(FALLING).add(5, 5, 0.1f);
    m_anims.at(FALLING).add(6, 5, 0.1f);
    m_anims.at(FALLING).add(7, 5, 0.1f);
    m_anims.at(FALLING).looped = false;

//  /* Death */
//  m_anims.emplace_back(texSize, 8, 9);
//  m_anims.back().add(0, 2, 0.2f);
//  m_anims.back().add(1, 2, 0.2f);
//  m_anims.back().add(2, 2, 0.2f);
//  m_anims.back().add(3, 2, 0.2f);
//  m_anims.back().add(4, 2, 0.2f);
//  m_anims.back().add(5, 2, 0.2f);
//  m_anims.back().add(6, 2, 0.2f);
//  m_anims.back().add(7, 2, 0.2f);
//  m_anims.back().looped = false;

    if (STATECOUNT != m_anims.size())
        std::cout << "Not all or too many animation states defined!"
                  << std::endl;
}

void Player::restartAnimsExcept(int index)
{
    for (auto& i : m_anims)
        // check if needs restart (?)
        if (i.first != index) i.second.restart();
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
            commands.push(m_actionbinds[SPRINT]);
        }
        break;
    case sf::Event::KeyReleased:
        if (event.key.code == getAssignedKey(SPRINT)) {
            commands.push(m_actionbinds[SPRINT]);
        }
        break;
    default: break;
    }
}

// For continuous real-time actions (WHILE an event happens)
void Player::handleInput(std::queue<Command>& commands)
{
    if (canJump) m_state = IDLE;

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
    case MOVE_RIGHT: // fallthrough
    case MOVE_UP:    // fallthrough
    case JUMP:       // fallthrough
    case MOVE_DOWN:
        return false;

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
