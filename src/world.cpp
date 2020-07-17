#include "world.h"
#include "entity.h"
#include "tile.h"
#include "player.h"
#include "command.h"
#include "playercmds.h"

//#define VIEW_HEIGHT 2560
//#define VIEW_WIDTH  1440
#define VIEW_HEIGHT 3200
#define VIEW_WIDTH  1800

// TODO(dan): tilemap.cpp
static const char* tiletexfile = "legacy_dungeon.png";
static f32 FORCE_TO_BREAK_HOLD = 50000000.f;
// std::unordered_map<u32, Tile*> tilemap;
bool g_cull_tiles = true;

void fillUpDemoQueue(std::queue<Command>& demoCmds);

World::World(const std::string& levelName, b32 menuMode)
  : m_view(sf::Vector2f(VIEW_WIDTH / 2, VIEW_HEIGHT / 2),
           sf::Vector2f(VIEW_HEIGHT, VIEW_WIDTH))
  , m_player(nullptr)
  , m_textures(".png")
  , m_levels(".png")
  , playerTileContact() // collision response
  , world(b2Vec2(0.f, 50.f))
  , menuMode(menuMode)
{
    // initialize nodes for every layer of the scene
    for (std::size_t i = 0; i < LAYER_COUNT; i++)
    {
        std::unique_ptr<Entity> layer(new Entity());
        m_layerNodes[i] = layer.get();
        m_scenegraph.attachChild(std::move(layer));
    }

    std::unique_ptr<Player> player(new Player(m_textures));
    m_player = player.get();
    m_layerNodes[LAYER_MID]->attachChild(std::move(player));

    if (!menuMode)
    {
        levelBuild(tilemap_mid, tilemap_back, &world, m_textures, m_levels,
                   maxMapSize, m_layerNodes, m_player, levelName,
                   "level_back.png");
    }
    else
    {
        levelBuild(tilemap_mid, tilemap_back, &world, m_textures, m_levels,
                   maxMapSize, m_layerNodes, m_player, levelName,
                   "menu_back.png");
    }

    m_view.setCenter(m_player->getPosition());
    world.SetContactListener(&playerTileContact);

    if (!menuMode)
    {
        DEBUG_GUI([]() { ImGui::Checkbox("TileMap Culling", &g_cull_tiles); });
        DEBUG_GUI([]() {
            ImGui::InputFloat("Force to break hold:", &FORCE_TO_BREAK_HOLD);
        });
    }

    //fillUpDemoQueue(demoCmds);

    // TODO(dan): test spawnbox
}

void World::update(f32 dt, sf::RenderWindow& window)
{
    { // VIEW UPDATE //////////////////////////////////////////////////////////
        // get player position on screen in pixels
        sf::Vector2u pPixelPos = (sf::Vector2u) window.mapCoordsToPixel(m_player->getPosition(),
                                                                        m_view);
        // get player position on screen in [0-1]
        sf::Vector2f pRelPos((f32) pPixelPos.x / window.getSize().x,
                             (f32) pPixelPos.y / window.getSize().y);

        f32 movefactor = 1.f;
        if (pRelPos.x < 0.3f) movefactor = 2.f;
        if (pRelPos.x > 0.7f) movefactor = 2.f;
        if (pRelPos.y < 0.3f) movefactor = 2.f;
        if (pRelPos.y > 0.7f) movefactor = 2.f;

        // move view if player gets out of borders
        // TODO breaks if player near border and zooming in/resizing
        // TODO(dan): camera class/.cpp?
        if (pRelPos.x < 0.4f && m_player->velocity.x < 0.f)
        {
            m_view.move(m_player->velocity.x * dt * movefactor, 0.f);
            m_view.setCenter(std::floor(m_view.getCenter().x), m_view.getCenter().y);
        }
        if (pRelPos.x > 0.6f && m_player->velocity.x > 0.f)
        {
            m_view.move(m_player->velocity.x * dt * movefactor, 0.f);
            m_view.setCenter(std::ceil(m_view.getCenter().x), m_view.getCenter().y);
        }
        if (pRelPos.y < 0.4f && m_player->velocity.y < 0.f)
        {
            m_view.move(0.f, m_player->velocity.y * dt * movefactor);
            m_view.setCenter(m_view.getCenter().x, std::floor(m_view.getCenter().y));
        }
        if (pRelPos.y > 0.6f && m_player->velocity.y > 0.f)
        {
            m_view.move(0.f, m_player->velocity.y * dt * movefactor);
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
        {
            m_player->facingRight ? m_player->velocity.x = 200.f
                                  : m_player->velocity.x = -200.f;
        }
    }
    else
    {
        m_player->velocity.x = 0.f;
    }

// testing "demo mode"
#if true
    if (menuMode)
    {
        if (!demoCmds.empty())
        {
            cmdQueue.push(demoCmds.front());
            demoCmds.pop();
        }
        else
        {
            fillUpDemoQueue(demoCmds);
        }
    }

    while (!cmdQueue.empty())
    {
        Command cmd = cmdQueue.front();
        cmdQueue.pop();
        // m_scenegraph.onCommand(cmd, dt); // bad performance w/ large // scenegraphs
        m_player->onCommand(cmd, dt);
    }

#else
    // serializing cmds
    Command serialCmd = nullCmd;

    while (!cmdQueue.empty())
    {
        Command cmd = cmdQueue.front();
        serialCmd = cmdQueue.front();
        cmdQueue.pop();
        // m_scenegraph.onCommand(cmd, dt); // bad performance w/ large
        // scenegraphs
        m_player->onCommand(cmd, dt);
    }

    cmdSerializer.push_back(serialCmd.cmdType);
#endif

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

    // leave player corpses when respawning
    /*
    if (m_player->leaveCorpse)
    {
        std::unique_ptr<Entity> corpse(new Corpse(*m_player->m_sprite.getTexture(),
                                                  m_player->facingRight));
        auto corpsePos = m_player->getPosition();
        corpse->setPosition(corpsePos);
        corpse->body = createBox(&world, corpsePos.x, corpsePos.y, 128 / 2, 128,
                                 b2_dynamicBody, corpse.get(), m_player, true);

        m_layerNodes[LAYER_MID]->attachChild(std::move(corpse));

        m_player->leaveCorpse = false;
    }
    */

    // Physics
    m_player->velocity.y += metersToPixels(world.GetGravity().y) * dt;
    m_player->body->SetLinearVelocity(b2Vec2(pixelsToMeters(m_player->velocity.x),
                                             pixelsToMeters(m_player->velocity.y)));
    world.Step(dt, (i32) 8, (i32) 3);

    m_scenegraph.update(dt);

    // workaround so y-velocity doesnt build up from gravity
    if (m_player->velocity.y > 1000.f) m_player->velocity.y = 1000.f;
}

void World::draw(sf::RenderWindow& window)
{
    window.setView(m_view);

    // TODO(dan): this maybe shouldn't happen in the drawloop, but putting it in
    // the update loop causes flickering...
    if (g_cull_tiles)
    { // TILEMAP CULLING ////////////////////////////////////////////////
        u32 view_left   = std::round( (m_view.getCenter().x - (m_view.getSize().x / 2)) / tile_width)  * tile_width;
        u32 view_top    = std::round( (m_view.getCenter().y - (m_view.getSize().y / 2)) / tile_height) * tile_height;
        u32 view_right  = std::round( (m_view.getCenter().x + (m_view.getSize().x / 2)) / tile_width)  * tile_width;
        u32 view_bottom = std::round( (m_view.getCenter().y + (m_view.getSize().y / 2)) / tile_height) * tile_height;

        for (u32 y = view_top; y <= view_bottom; y += tile_height)
        {
            for (u32 x = view_left; x <= view_right; x += tile_width)
            {
                u32 id = levelTileIDfromCoords(x, y, maxMapSize);
                if (tilemap_mid.find(id) != tilemap_mid.end())
                    tilemap_mid[id]->shouldDraw = true;
                if (tilemap_back.find(id) != tilemap_back.end())
                    tilemap_back[id]->shouldDraw = true;
            }
        }
    } ////////////////////////////////////////////////////////////////////

    window.draw(m_scenegraph);
}

void World::spawnBox(sf::Vector2f pos, b32 isStatic)
{
    levelPlaceBox(pos, isStatic, &world, tilemap_mid, m_layerNodes, maxMapSize,
                  m_textures.get(tiletexfile), m_player);
}

void fillUpDemoQueue(std::queue<Command>& demoCmds)
{
    //return;
    for (int i = 0; i < 50; i++)
        demoCmds.push(nullCmd);
    for (int i = 0; i < 230; i++)
        demoCmds.push(moveRightCmd);
    demoCmds.push(jumpCmd);
    for (int i = 0; i < 50; i++)
        demoCmds.push(moveRightCmd);
    demoCmds.push(jumpCmd);
    for (int i = 0; i < 50; i++)
        demoCmds.push(moveRightCmd);
    for (int i = 0; i < 100; i++)
        demoCmds.push(nullCmd);
    for (int i = 0; i < 350; i++)
        demoCmds.push(moveLeftCmd);

    /*
    std::string democmdtype;
    u32 type;
    std::ifstream democommands;
    democommands.open("democommands.txt");
    if (democommands.is_open())
    {
        while (std::getline(democommands, democmdtype))
        {
            std::cout << democmdtype << std::endl;
            demoCmds.push(cmdFromType[std::stoi(democmdtype)]);
        }
    }
    democommands.close();
    */
}
