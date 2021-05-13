#include "menu.h"
#include "game.h"
#include "gui.h"
#include "levelgen.h"
#include "world.h"
#include "entity.h"

// menu items
static std::shared_ptr<GUIContainer> guiContainer;
static std::shared_ptr<GUIButton> guiButton1;
static std::shared_ptr<GUIButton> guiButton2;
static std::shared_ptr<GUIButton> guiButton3;
static std::shared_ptr<GUIContainer> titleContainer;
static std::shared_ptr<GUILabel> titleLabel;
static Game* gameInst;

static World world("menu_mid.png", true);

void menuInit(Game* game, ResourcePool<sf::Texture>& textures,
              ResourcePool<sf::Font>& fonts)
{
    gameInst = game;

    // MENU
    guiContainer = std::make_shared<GUIContainer>();
    titleContainer = std::make_shared<GUIContainer>();

    guiButton1 = std::make_shared<GUIButton>(fonts, textures);
    guiButton1->setText("START GAME");
    guiButton1->setCallback([&]() {
        gameInst->currentState = IN_GAME;
        gameInst->m_world.m_player->retry();
        for (auto corpse : gameInst->m_world.m_player->lyingCorpses)
            corpse->exists = false;
    });
    guiButton1->setPosition(100, 100);

    guiButton2 = std::make_shared<GUIButton>(fonts, textures);
    guiButton2->setText("SETTINGS");
    guiButton2->setCallback([&]() { });
    guiButton2->setPosition(100, 250);

    guiButton3 = std::make_shared<GUIButton>(fonts, textures);
    guiButton3->setText("END GAME");
    guiButton3->setCallback([&]() { gameInst->currentState = EXIT_GAME; });
    guiButton3->setPosition(100, 400);

    guiContainer->pack(guiButton1);
    guiContainer->pack(guiButton2);
    guiContainer->pack(guiButton3);

    guiContainer->setPosition(160, 360);

    titleLabel = std::make_shared<GUILabel>("CHECKBOX", fonts);
    //titleLabel->setColor(sf::Color(80,40,1));
    titleContainer->pack(titleLabel);
    titleContainer->setPosition(200,250);
}

void menuWorldUpdate(f32 dt, sf::RenderWindow& window)
{
    world.update(dt, window);
}

void menuHandleInput()
{
    world.handleInput();
}

void menuHandleEvents(sf::Event event)
{
    world.handleEvents(event);
    guiContainer->handleEvent(event);
}

void menuDraw(sf::RenderWindow& window)
{
    window.setView(world.m_view);
    world.draw(window);
    window.setView(window.getDefaultView());
    window.draw(*guiContainer);
    window.draw(*titleContainer);
}
