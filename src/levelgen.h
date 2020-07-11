#pragma once
#include "pch.h"

template<typename Resource> class ResourcePool;
class Player;
class Tile;
class Entity;

enum Layer
{
    LAYER_BACK,
    LAYER_MID,
    LAYER_FORE,
    LAYER_COUNT
};

void buildLevel(std::map<u32, Tile*>& tilemap, b2World* world,
                ResourcePool<sf::Texture>& textures,
                ResourcePool<sf::Image>& levels, sf::Vector2u& maxMapSize,
                std::array<Entity*, LAYER_COUNT>& m_layerNodes, Player* player,
                std::string levelName = "");

b2Body* createBox(b2World* world, i32 posX, i32 posY, i32 sizeX, i32 sizeY,
                  b2BodyType type, void* userData, Player* player,
                  b32 collidable = true);

u32 tileIDfromCoords(const u32 x, const u32 y, const sf::Vector2u MaxMapSize);
