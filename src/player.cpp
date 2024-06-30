#include "player.h"
#include "resourcepool.h"
#include "physics.h"
#include "playercmds.h"

// Left X Axis:  sf::Joystick::Axis::X
// Left Y Axis:  sf::Joystick::Axis::Y
// Right X Axis: sf::Joystick::Axis::U
// Right Y Axis: sf::Joystick::Axis::V
// L Trigger:    sf::Joystick::Axis::Z
// R Trigger:    sf::Joystick::Axis::R
// D-Pad X Axis: sf::Joystick::Axis::PovX
// D-Pad Y Axis: sf::Joystick::Axis::PovY
enum DS4Button
{
    DS4_CROSS,
    DS4_CIRCLE,
    DS4_TRIANGLE,
    DS4_SQUARE,
    DS4_L1,
    DS4_R1,
    DS4_L2,
    DS4_R2,
    DS4_SELECT,
    DS4_START,
    DS4_HOME,
    DS4_L3,
    DS4_R3
};

static const float DISTANCE_TO_HOLD = 25000.f;

static u32 playercount = 0; // workaround

Player::Player(ResourcePool<sf::Texture>& textures)
  : m_sprite(textures.get("platformer_sprites_base.png"))
{
    EventSystem::subscribe(std::bind(&Player::processEvent, this, std::placeholders::_1));

    m_state = IDLE;
    m_sprite.setTextureRect(sf::IntRect(0, 0, PLAYER_WIDTH, PLAYER_HEIGHT));
    m_sprite.setOrigin(64, 64);
    createAnimations();

    assignKey(sf::Keyboard::A, MOVE_LEFT);
    assignKey(sf::Keyboard::D, MOVE_RIGHT);
    assignKey(sf::Keyboard::Space, JUMP);
    assignKey(sf::Keyboard::F, HOLD);
    assignKey(sf::Keyboard::Y, WINNING);
    assignKey(sf::Keyboard::R, RESPAWN);
    assignKey(sf::Keyboard::BackSpace, RETRY);
    assignButton(DS4_CROSS, JUMP);
    assignButton(DS4_TRIANGLE, RESPAWN);
    assignButton(DS4_SQUARE, HOLD);
    assignButton(DS4_START, RETRY);

    m_actionbinds[MOVE_LEFT]  = moveLeftCmd;
    m_actionbinds[MOVE_RIGHT] = moveRightCmd;
    m_actionbinds[JUMP]       = jumpCmd;
    m_actionbinds[WINNING]    = winCmd;
    m_actionbinds[RESPAWN]    = respawnCmd;
    m_actionbinds[HOLD]       = holdCmd;
    m_actionbinds[RETRY]      = retryCmd;

    // TODO(dan): sort through holdables, remove holdables too far away
    set_comparator = [&](const Entity* e1, const Entity* e2) {
        auto e1_to_p = e1->getPosition() - this->getPosition();
        auto e2_to_p = e2->getPosition() - this->getPosition();
        f32 e1_dist = std::pow(e1_to_p.x, 2) + std::pow(e1_to_p.y, 2);
        f32 e2_dist = std::pow(e2_to_p.x, 2) + std::pow(e2_to_p.y, 2);
        return e1_dist < e2_dist;
    };

    holdables = std::set<Entity*, decltype(set_comparator)>(set_comparator);

    if (playercount++ > 0)
    {
        DEBUG_GUI([&]() { ImGui::Text("PLAYER_STATE: %d", m_state); });
        DEBUG_GUI([&]() { ImGui::Text("GOLDCOUNT: %d", goldCount); });
        DEBUG_GUI([&]() {
            ImGui::Text("current set of holdables:");
            for (auto it = holdables.begin(); it != holdables.end(); ++it)
            {
                ImGui::Text("%.p", *it);
            }
        });
    }
}

// TODO testing eventsystem
void Player::processEvent(const Event& evn)
{
    switch (evn.type)
    {
        case EventType::EVENT_PLAYER_COIN_PICKUP:
            auto coin = (Coin*) evn.args.at("coin");
            goldCount += coin->value;
            collectedCoins.push_back(coin);
            coin->collected = true;
            //evn.handled = true; // shouldn't be set, bc there are 2 players
            break;
    }
}

/*
 * TODO: set/unset booleans w/ KeyPressed/KeyReleased ?
 * TODO: don't read keyinputs if window not focused
 */
void Player::updateCurrent(f32 dt)
{
    auto time = celebTimer.getElapsedTime();
    if (gameWon)
    {
        m_sprite.setTextureRect(m_anims.at(CELEBRATING).update(dt));
        if (time.asSeconds() > 5.f)
        {
            gameOver = true;
            gameWon = false;
            celebTimer.restart();
        }
        return;
    }

    // apply physics simulation position to sprite
    // setRotation(radToDeg(body->GetAngle()));
    setPosition(metersToPixels(body->GetPosition().x), metersToPixels(body->GetPosition().y));

    if (velocity.y > 100.f && !dead) m_state = FALLING;

    // Animation:
    // only update texture if animation was found
    if (m_anims.find(m_state) != m_anims.end()) {
        restartAnimsExcept(m_state);
        m_anims.at(m_state).flipped = !facingRight;
        m_sprite.setTextureRect(m_anims.at(m_state).update(dt));
    }

    // iterate through holdabels, check distance to player, if above
    // min_hold_distance, remove out of set
    for (auto it = holdables.cbegin(); it != holdables.cend(); it++)
    {
        // TODO(dan): paint nearest holdable red/blue/...
        /*
        static auto first = holdables.begin();
        ((Tile*)(*first))->m_sprite.setColor(sf::Color::Blue);
        */

        auto ent_to_p = (*it)->getPosition() - this->getPosition();
        f32 ent_dist = std::pow(ent_to_p.x, 2) + std::pow(ent_to_p.y, 2);
        if (ent_dist > DISTANCE_TO_HOLD)
        {
            holdables.erase(it, holdables.cend());
            return;
        }
        else
        {
            continue;
        }
    }

    if (holding)
    {
        auto box_pos = getPosition() + (50.f * forwardRay());
        //((Tile*) holding)->m_sprite.setColor(sf::Color::Red);

        //((Tile*) holding)->body->SetTransform(b2Vec2(pixelsToMeters(box_pos.x), pixelsToMeters(box_pos.y)), 0);
        /*
        holding->setRotation(radToDeg(body->GetAngle()));
        holding->setPosition(metersToPixels(body->GetPosition().x),
                             metersToPixels(body->GetPosition().y));
        */
    }

    // TODO(dan): hardcoded
    // player needs to respawn
    if (dead) m_state = DEAD; // TODO(dan): play death animation
}

void Player::createAnimations()
{
    sf::Vector2u texSize = m_sprite.getTexture()->getSize();
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

    /* Death */
    m_anims.emplace(DEAD, Animation(texSize, 8, 9));
    m_anims.at(DEAD).add(0, 2, 0.08f);
    m_anims.at(DEAD).add(1, 2, 0.08f);
    m_anims.at(DEAD).add(2, 2, 0.08f);
    m_anims.at(DEAD).add(3, 2, 0.08f);
    m_anims.at(DEAD).add(4, 2, 0.08f);
    m_anims.at(DEAD).add(5, 2, 0.08f);
    m_anims.at(DEAD).add(6, 2, 0.08f);
    m_anims.at(DEAD).add(7, 2, 0.08f);
    m_anims.at(DEAD).looped = false;

    /* Celebrate */
    m_anims.emplace(CELEBRATING, Animation(texSize, 8, 9));
    m_anims.at(CELEBRATING).add(0, 3, 0.1f);
    m_anims.at(CELEBRATING).add(1, 3, 0.1f);
    m_anims.at(CELEBRATING).add(2, 3, 0.1f);
    m_anims.at(CELEBRATING).add(3, 3, 0.1f);
    m_anims.at(CELEBRATING).add(3, 3, 0.1f);
    m_anims.at(CELEBRATING).add(2, 3, 0.1f);
    m_anims.at(CELEBRATING).add(1, 3, 0.1f);
    m_anims.at(CELEBRATING).looped = true;

    // Not all or too many animation states defined
    assert(STATECOUNT == m_anims.size());
}

void Player::restartAnimsExcept(i32 index)
{
    for (auto& i : m_anims)
        // check if needs restart (?)
        if (i.first != index) i.second.restart();
}

void Player::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_sprite, states);
}

// For one-time actions (WHEN an event happens)
void Player::handleEvent(const sf::Event& event, std::queue<Command>& commands)
{
    if (dead) return;

    switch (event.type)
    {
    case sf::Event::KeyPressed:
        for (auto i : m_keybinds)
            if (event.key.code == getAssignedKey(i.second))
                commands.push(m_actionbinds[i.second]);
        break;

    case sf::Event::JoystickButtonPressed:
        for (auto i : m_joybinds)
        {
            if (sf::Joystick::isButtonPressed(0, i.first) &&
                isOneShot(i.second))
                commands.push(m_actionbinds[i.second]);
        }
        break;

    default: break;
    }
}

// For continuous real-time actions (WHILE an event happens)
void Player::handleInput(std::queue<Command>& commands)
{
    // TODO(dan): hardcoded
    // only respond to respawn key if dead
    if (dead)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
            commands.push(m_actionbinds[RESPAWN]);
        if (sf::Joystick::isButtonPressed(0, DS4_TRIANGLE))
            commands.push(m_actionbinds[RESPAWN]);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace))
            commands.push(m_actionbinds[RETRY]);
        if (sf::Joystick::isButtonPressed(0, DS4_START))
            commands.push(m_actionbinds[RETRY]);
        return;
    }

    if (canJump) m_state = IDLE;

    for (auto i : m_keybinds)
        if (sf::Keyboard::isKeyPressed(i.first) && !isOneShot(i.second))
            commands.push(m_actionbinds[i.second]);

    for (auto i : m_joybinds)
        if (sf::Joystick::isButtonPressed(0, i.first) && !isOneShot(i.second))
            commands.push(m_actionbinds[i.second]);

    // TODO(dan): hardcoded input for joystick axis
    if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) < 0)
        commands.push(m_actionbinds[MOVE_LEFT]);
    if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) > 0)
        commands.push(m_actionbinds[MOVE_RIGHT]);
}

b32 Player::isOneShot(Action action)
{
    switch (action) {
    case MOVE_LEFT:  // fallthrough
    case MOVE_RIGHT: // fallthrough
    case MOVE_UP:    // fallthrough
    case JUMP:       // fallthrough
    case MOVE_DOWN:  // fallthrough
    case WINNING:    // fallthrough
    case DYING:      // fallthrough
        return false;

    default: return true;
    }
}

void Player::assignKey(sf::Keyboard::Key key, Action action)
{
    // Remove all keys that already map to action
    for (auto it = m_keybinds.begin(); it != m_keybinds.end();)
    {
        if (it->second == action) m_keybinds.erase(it++);
        else ++it;
    }

    // Insert new binding
    m_keybinds[key] = action;
}

void Player::assignButton(u32 button, Action action)
{
    // Remove all keys that already map to action
    for (auto it = m_joybinds.begin(); it != m_joybinds.end();)
    {
        if (it->second == action) m_joybinds.erase(it++);
        else ++it;
    }

    // Insert new binding
    m_joybinds[button] = action;
}

sf::Keyboard::Key Player::getAssignedKey(Action action) const
{
    for (auto i : m_keybinds)
        if (i.second == action) return i.first;

    return sf::Keyboard::Unknown;
}

u32 Player::getAssignedButton(Action action) const
{
    for (auto i : m_joybinds)
        if (i.second == action) return i.first;

    return 10; // no joystick::unknown (?)
}

void Player::retry()
{
    body->SetTransform(b2Vec2(pixelsToMeters(spawn_loc.x),
                              pixelsToMeters(spawn_loc.y)),
                       0);
    for (auto& cb : checkboxes)
    {
        auto newpos = b2Vec2(pixelsToMeters(cb.origPos.x),
                             pixelsToMeters(cb.origPos.y));
        cb.box->body->SetLinearVelocity(b2Vec2(0,0));
        cb.box->body->SetTransform(newpos, 0);
    }

    for (auto c : collectedCoins)
        c->collected = false;

    collectedCoins.clear();
    goldCount = 0;
    holding = nullptr;
    dead = false;
}
