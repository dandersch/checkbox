#include "world.h"
#include "entity.h"
#include "tile.h"
#include "player.h"
#include "command.h"
#include "levelgen.h"

#define VIEW_HEIGHT 1280
#define VIEW_WIDTH  720

static Entity m_scenegraph;
static std::array<Entity*, LAYER_COUNT> m_layerNodes;
static ResourcePool<sf::Texture> m_textures(".png");
static ResourcePool<sf::Image> m_levels(".png");
static b2World world(b2Vec2(0.f, 25.f)); // TODO keep using higher gravity (?)
static sf::Vector2u maxMapSize;

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
            // direction change allowed when standing still
            if (p.velocity.x == 0.f) p.facingRight = rightDir;
            return;
        }

        if (!p.holding) {
            p.velocity += (1.5f * velocity);
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

// TODO(dan): tilemap.cpp
static const char* tiletexfile = "simple_tiles_32x32.png";
static f32 FORCE_TO_BREAK_HOLD = 2500000.f;
//std::unordered_map<u32, Tile*> tilemap;
static std::map<u32, Tile*> tilemap;
bool g_cull_tiles = true;
enum TileSheet
{
    TILE_SIZE = 32,
    SHEET_WIDTH = 288,
    SHEET_HEIGHT = 64
};

bool create_joint = false;
b2Body* body_player = nullptr;
b2WheelJoint* hold_joint = nullptr;

static std::queue<Command> demoCmds;
void fillUpDemoQueue()
{
    Command moveRight;
    moveRight.action = derivedAction<Player>(PlayerMover(100.f, 0.f, true));
    moveRight.category = ENTITY_PLAYER;
    Command moveLeft;
    moveLeft.action = derivedAction<Player>(PlayerMover(-100.f, 0.f, false));
    moveLeft.category = ENTITY_PLAYER;
    Command jump;
    jump.category = ENTITY_PLAYER;
    jump.action = derivedAction<Player>([](Player& p, f32) {
        if (p.canJump && p.holding)
        {
            p.fixedJump = true;
            p.velocity.y = -sqrtf(2.0f * 981.f * 100.f);
            p.m_state = Player::JUMPING;
            p.canJump = false;
        }
        else if (p.canJump)
        {
            p.velocity.y = -sqrtf(2.0f * 981.f * 120.f);
            p.m_state = Player::JUMPING;
            p.canJump = false;
        }
    });

    for (int i = 0; i < 100; i++)
        demoCmds.push(moveRight);
    demoCmds.push(jump);
    for (int i = 0; i < 100; i++)
        demoCmds.push(moveRight);
    for (int i = 0; i < 100; i++)
        demoCmds.push(moveLeft);
    demoCmds.push(jump);
    for (int i = 0; i < 100; i++)
        demoCmds.push(moveLeft);
}

World::World(sf::RenderWindow& window)
  : m_window(window)
  , m_view(sf::Vector2f(640.f, 360.f), sf::Vector2f(VIEW_HEIGHT, VIEW_WIDTH))
  , m_player(nullptr)
  , playerTileContact()      // collision response
{
    // initialize nodes for every layer of the scene
    for (std::size_t i = 0; i < LAYER_COUNT; i++) {
        std::unique_ptr<Entity> layer(new Entity());
        m_layerNodes[i] = layer.get();
        m_scenegraph.attachChild(std::move(layer));
    }

    std::unique_ptr<Player> player(new Player(m_textures));
    m_player = player.get();
    m_layerNodes[LAYER_MID]->attachChild(std::move(player));

    buildLevel(tilemap, &world, m_textures, m_levels, maxMapSize, m_layerNodes,
               m_player, "level0.png");

    m_view.setCenter(m_player->getPosition());
    world.SetContactListener(&playerTileContact);

    DEBUG_GUI([]() { ImGui::Checkbox("TileMap Culling", &g_cull_tiles); });
    DEBUG_GUI([]() { ImGui::InputFloat("Force to break hold:", &FORCE_TO_BREAK_HOLD);} );

    fillUpDemoQueue();
}

void World::update(f32 dt)
{
    { // VIEW UPDATE //////////////////////////////////////////////////////////
        // get player position on screen in pixels
        sf::Vector2u pPixelPos = (sf::Vector2u) m_window.mapCoordsToPixel(m_player->getPosition(),
                                                                          m_view);
        // get player position on screen in [0-1]
        sf::Vector2f pRelPos((f32) pPixelPos.x / m_window.getSize().x,
                             (f32) pPixelPos.y / m_window.getSize().y);

        // move view if player gets out of borders
        // TODO breaks if player near border and zooming in/resizing
        // TODO(dan): camera class/.cpp?
        if (pRelPos.x < 0.3f && m_player->velocity.x < 0.f)
        {
            m_view.move(m_player->velocity.x * dt, 0.f);
            m_view.setCenter(std::floor(m_view.getCenter().x), m_view.getCenter().y);
        }
        if (pRelPos.x > 0.7f && m_player->velocity.x > 0.f)
        {
            m_view.move(m_player->velocity.x * dt, 0.f);
            m_view.setCenter(std::ceil(m_view.getCenter().x), m_view.getCenter().y);
        }
        if (pRelPos.y < 0.3f && m_player->velocity.y < 0.f)
        {
            m_view.move(0.f, m_player->velocity.y * dt);
            m_view.setCenter(m_view.getCenter().x, std::floor(m_view.getCenter().y));
        }
        if (pRelPos.y > 0.7f && m_player->velocity.y > 0.f)
        {
            m_view.move(0.f, m_player->velocity.y * dt);
            m_view.setCenter(m_view.getCenter().x, std::ceil(m_view.getCenter().y));
        }

        // workaround to let player not get stuck outside of viewborders
        if (pRelPos.x < 0.05f) m_view.setCenter(m_player->getPosition());
        if (pRelPos.x > 0.95f) m_view.setCenter(m_player->getPosition());
        if (pRelPos.y < 0.05f) m_view.setCenter(m_player->getPosition());
        if (pRelPos.y > 0.95f) m_view.setCenter(m_player->getPosition());

        m_view.setCenter(std::round(m_view.getCenter().x), std::round(m_view.getCenter().y));
    } //////////////////////////////////////////////////////////////////////////

    // reset velocity unless fixed jumping
    if (m_player->fixedJump) // TODO(dan): find a better way
    {
        if (m_player->velocity.x != 0)
            m_player->facingRight ? m_player->velocity.x = 200.f
                                  : m_player->velocity.x = -200.f;
    }
    else
    {
        m_player->velocity.x = 0.f;
    }

    // testing "demo mode"
    if (!demoCmds.empty())
    {
        cmdQueue.push(demoCmds.front());
        demoCmds.pop();
    } else {
        fillUpDemoQueue();
    }

    while (!cmdQueue.empty()) {
        Command cmd = cmdQueue.front();
        cmdQueue.pop();
        //m_scenegraph.onCommand(cmd, dt); // bad performance w/ large scenegraphs
        m_player->onCommand(cmd, dt);
    }

    if (m_player->holding)
    {
        if (hold_joint)
        {
            // check if box is facing right side
            auto dir = m_player->holding->body->GetPosition() - m_player->body->GetPosition();
            if ((dir.x > 0 && m_player->forwardRay().x < 0) ||
                (dir.x < 0 && m_player->forwardRay().x > 0))
            {
                auto box_pos = m_player->getPosition() +
                               (35.f * m_player->forwardRay());
                m_player->holding->body->SetTransform(b2Vec2(pixelsToMeters(box_pos.x),
                                                             pixelsToMeters(box_pos.y)),
                                                      0);
            }

            // destroy joint if force too great
            b2Vec2 reactionForce = hold_joint->GetReactionForce(1 / dt);
            float forceModuleSq = reactionForce.LengthSquared();
            if (forceModuleSq > FORCE_TO_BREAK_HOLD)
            {
                world.DestroyJoint(hold_joint);
                hold_joint = nullptr;
                m_player->holding = nullptr;
            }
        }
        else // create joint
        {
            b2WheelJointDef jointDef;
            jointDef.bodyA = m_player->body;
            jointDef.bodyB = m_player->holding->body;
            jointDef.collideConnected = true;

            // create the joint
            hold_joint = (b2WheelJoint*) world.CreateJoint(&jointDef);
        }
    }
    else
    {
        if (hold_joint)
        {
            world.DestroyJoint(hold_joint);
            hold_joint = nullptr;
        }
    }

    // Physics
    m_player->velocity.y += metersToPixels(world.GetGravity().y) * dt;
    m_player->body->SetLinearVelocity(b2Vec2(pixelsToMeters(m_player->velocity.x),
                                             pixelsToMeters(m_player->velocity.y)));
    world.Step(dt, (i32) 8, (i32) 3);

    m_scenegraph.update(dt);

    // workaround so y-velocity doesnt build up from gravity
    if (m_player->velocity.y > 2000.f) m_player->velocity.y = 2000.f;
}

void World::draw()
{
    m_window.setView(m_view);

    // TODO(dan): this maybe shouldn't happen in the drawloop, but putting it in
    // the update loop causes flickering...
    if (g_cull_tiles)
    { // TILEMAP CULLING ////////////////////////////////////////////////
        u32 view_left   = std::round( (m_view.getCenter().x - (m_view.getSize().x / 2)) / 32) * 32;
        u32 view_top    = std::round( (m_view.getCenter().y - (m_view.getSize().y / 2)) / 32) * 32;
        u32 view_right  = std::round( (m_view.getCenter().x + (m_view.getSize().x / 2)) / 32) * 32;
        u32 view_bottom = std::round( (m_view.getCenter().y + (m_view.getSize().y / 2)) / 32) * 32;

        for (u32 y = view_top; y <= view_bottom; y += 32)
        {
            for (u32 x = view_left; x <= view_right; x += 32)
            {
                u32 id = tileIDfromCoords(x, y, maxMapSize);
                if (tilemap.find(id) != tilemap.end())
                {
                    tilemap[id]->shouldDraw = true;

                    auto& tile = tilemap[id];
                    // collision filtering (doesn't seem to improve performance)
                    /*
                    // First approach
                    // NOTE(dan): also doesn't set back maskbits when tile is
                    // out of view
                    b2Fixture* fixt = tile->body->GetFixtureList();
                    b2Filter filter = fixt->GetFilterData();
                    filter.maskBits = 0xFFFF;
                    fixt->SetFilterData(filter);
                    */

                    // Second approach
                    // tile->body->SetActive(true);
                }
            }
        }
    } ////////////////////////////////////////////////////////////////////

    m_window.draw(m_scenegraph);
}

void World::spawnBox(sf::Vector2f pos, b32 isStatic)
{
    u32 tilenr = 1;
    b2BodyType type = b2_dynamicBody;
    if (isStatic)
    {
        type = b2_staticBody;
        tilenr = 2;
    }

    // clamp boxes to tileraster
    auto xpos = std::round(pos.x / 32) * 32;
    auto ypos = std::round(pos.y / 32) * 32;

    // there already is a collidable tile here so don't spawn another one
    auto tileIt = tilemap.find(tileIDfromCoords(xpos, ypos, maxMapSize));
    if (tileIt != tilemap.end())
    {
        if ((*tileIt).second->body->IsActive()) return;
    }

    std::unique_ptr<Tile> box(new Tile(m_textures.get(tiletexfile),
                                                   sf::IntRect(tilenr * 32, 0  * 32,
                                                               32, 32)));
    box->moving = true;
    box->shouldDraw = true;
    box->setPosition(xpos, ypos);

    // testing dynamic body as checkpoint
    if (isStatic) box->typeflags = ENTITY_TILE;
    else box->typeflags = ENTITY_TILE | ENTITY_CHECKPOINT | ENTITY_HOLDABLE;

    box->body = createBox(&world, xpos, ypos, 32, 32, type, box.get(), m_player);

    box->body->SetFixedRotation(true);
    m_layerNodes[LAYER_MID]->attachChild(std::move(box));
}
