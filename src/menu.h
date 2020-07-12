#pragma once
#include "pch.h"

template<typename Resource>
class ResourcePool;
class Game;

void menuInit(Game* game, ResourcePool<sf::Texture>& textures,
              ResourcePool<sf::Font>& fonts);
void menuWorldUpdate(f32 dt, sf::RenderWindow& window);
void menuHandleInput();
void menuHandleEvents(sf::Event event);
void menuDraw(sf::RenderWindow& window);
