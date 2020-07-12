#include "menu.h"
#include "game.h"
#include "gui.h"

static std::shared_ptr<GUIContainer> guiContainer;
static std::shared_ptr<GUIButton> guiButton1;
static std::shared_ptr<GUIButton> guiButton2;
static Game* gameInst;

void menuInit(Game* game, ResourcePool<sf::Texture>& textures,
              ResourcePool<sf::Font>& fonts)
{
    gameInst = game;

    // MENU
    guiContainer = std::make_shared<GUIContainer>();
    guiButton1 = std::make_shared<GUIButton>(fonts, textures);
    guiButton1->setText("START GAME");
    guiButton1->setCallback([&]() { gameInst->currentState = IN_GAME; });
    guiButton1->setPosition(100, 100);
    guiButton2 = std::make_shared<GUIButton>(fonts, textures);
    guiButton2->setText("END GAME");
    guiButton2->setCallback([&]() { gameInst->currentState = EXIT_GAME; });
    guiButton2->setPosition(100, 250);
    guiContainer->pack(guiButton1);
    guiContainer->pack(guiButton2);
    guiContainer->setPosition(640, 360);
}

void menuHandleEvents(sf::Event event)
{
    guiContainer->handleEvent(event);
}

void menuDraw(sf::RenderWindow& window)
{
    window.draw(*guiContainer);
}
