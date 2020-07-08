#include "player.h"
#include "resourcepool.h"
#include "physics.h"

// functor for player movement
struct PlayerMover
{
    PlayerMover(f32 vx, f32 vy, b32 rightDir = false)
      : velocity(vx, vy)
      , rightDir(rightDir)
    {}
    void operator()(Player& p, f32) const
    {
        // movement not controllable mid-jump holding a box
        if ((p.m_state == Player::JUMPING ||
             p.m_state == Player::FALLING) && p.holding)
        {
            return;
        }

        if (p.running) {
            p.velocity += (2.f * velocity);
            if (p.canJump) p.m_state = Player::RUNNING;
        } else {
            p.velocity += velocity;
            if (p.canJump) p.m_state = Player::WALKING;
        }

        p.facingRight = rightDir;
    }

    sf::Vector2f velocity;
    b32 rightDir;
};

Player::Player(ResourcePool<sf::Texture>& textures)
  : m_sprite(textures.get("platformer_sprites_base.png"))
  , speed(100.f)
{
    m_state = IDLE;
    m_sprite.setTextureRect(sf::IntRect(0, 0, PLAYER_WIDTH, PLAYER_HEIGHT));
    m_sprite.setOrigin(32,32);
    createAnimations();

    assignKey(sf::Keyboard::A, MOVE_LEFT);
    assignKey(sf::Keyboard::D, MOVE_RIGHT);
    assignKey(sf::Keyboard::LShift, SPRINT);
    assignKey(sf::Keyboard::Space, JUMP);
    assignKey(sf::Keyboard::F, HOLD);
    assignKey(sf::Keyboard::X, DYING);
    assignKey(sf::Keyboard::R, RESPAWN);

    m_actionbinds[MOVE_LEFT].action  = derivedAction<Player>(PlayerMover(-speed, 0.f, false));
    m_actionbinds[MOVE_RIGHT].action = derivedAction<Player>(PlayerMover(+speed, 0.f, true));
    m_actionbinds[SPRINT].action = derivedAction<Player>([](Player& p, f32) {
        if (p.holding) p.running = false;
        else p.running = !p.running;
    });
    m_actionbinds[JUMP].action = derivedAction<Player>([](Player& p, f32) {
        if (p.canJump && p.holding)
        {
            p.fixedJump = true;
            p.velocity.y = -sqrtf(2.0f * 981.f * 120.f);
            p.m_state = JUMPING;
            p.canJump = false;
        }
        else if (p.canJump)
        {
            p.velocity.y = -sqrtf(2.0f * 981.f * 120.f);
            p.m_state = JUMPING;
            p.canJump = false;
        }
    });
    m_actionbinds[DYING].action = derivedAction<Player>([](Player& p, f32) { p.m_state = DEAD; });
    m_actionbinds[RESPAWN].action = derivedAction<Player>([](Player& p, f32) {
        if (p.checkpoint_box == p.holding && !p.dead) return; // holding the checkpoint
        if (p.checkpoint_box)
            p.body->SetTransform(b2Vec2(pixelsToMeters(p.checkpoint_box->getPosition().x),
                                        pixelsToMeters(p.checkpoint_box->getPosition().y)), 0);
        else
            p.body->SetTransform(b2Vec2(pixelsToMeters(p.spawn_loc.x),
                                        pixelsToMeters(p.spawn_loc.y)), 0);

        p.dead = false;
    });

    m_actionbinds[HOLD].action = derivedAction<Player>([](Player& p, f32) {
        if (p.holding && !p.dead)
        {
            ((Tile*) p.holding)->m_sprite.setColor(sf::Color::White);
            p.holding = nullptr;
            // TODO apply force
        }
        else
        {
            p.holding = *p.holdables.begin();
        }
    });

    for (auto& i : m_actionbinds) i.second.category = ENTITY_PLAYER;

    DEBUG_GUI([&]() { ImGui::Text("PLAYER_STATE: %d", m_state); });

    // TODO(dan): sort through holdables, remove holdables too far away
    set_comparator = [&](const Entity* e1, const Entity* e2) {
        auto e1_to_p = e1->getPosition() - this->getPosition();
        auto e2_to_p = e2->getPosition() - this->getPosition();
        f32 e1_dist = std::pow(e1_to_p.x, 2) + std::pow(e1_to_p.y, 2);
        f32 e2_dist = std::pow(e2_to_p.x, 2) + std::pow(e2_to_p.y, 2);
        return e1_dist < e2_dist;
    };

    holdables = std::set<Entity*, decltype(set_comparator)>(set_comparator);

    DEBUG_GUI([&]() {
        ImGui::Text("current set of holdables:");
        for (auto it = holdables.begin(); it != holdables.end(); ++it)
        {
            ImGui::Text("%.p", *it);
        }
    });
}

/*
 * TODO: set/unset booleans w/ KeyPressed/KeyReleased ?
 * TODO: don't read keyinputs if window not focused
 */
void Player::updateCurrent(f32 dt)
{
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
        if (ent_dist > 10000.f)
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
        auto box_pos = getPosition() + (35.f * forwardRay());
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
    // workaround to support modifiers
    switch (event.type)
    {
    case sf::Event::KeyPressed:
        if (event.key.code == getAssignedKey(SPRINT))
            commands.push(m_actionbinds[SPRINT]);
        if (event.key.code == getAssignedKey(HOLD))
            commands.push(m_actionbinds[HOLD]);
        if (event.key.code == getAssignedKey(RESPAWN))
            commands.push(m_actionbinds[RESPAWN]);
        break;
    case sf::Event::KeyReleased:
        if (event.key.code == getAssignedKey(SPRINT))
            commands.push(m_actionbinds[SPRINT]);
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
        return;
    }

    if (canJump) m_state = IDLE;

    for (auto i : m_keybinds)
        if (sf::Keyboard::isKeyPressed(i.first) && !isOneShot(i.second))
            commands.push(m_actionbinds[i.second]);
}

b32 Player::isOneShot(Action action)
{
    switch (action) {
    case MOVE_LEFT:  // fallthrough
    case MOVE_RIGHT: // fallthrough
    case MOVE_UP:    // fallthrough
    case JUMP:       // fallthrough
    case MOVE_DOWN:
    case DYING:
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
