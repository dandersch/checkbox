#pragma once
#include "pch.h"

template<typename Resource>
class ResourcePool;
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

void levelBuild(std::map<u32, Tile*>& tilemap, b2World* world,
                ResourcePool<sf::Texture>& textures,
                ResourcePool<sf::Image>& levels, sf::Vector2u& maxMapSize,
                std::array<Entity*, LAYER_COUNT>& m_layerNodes, Player* player,
                const std::string& levelName = "");

// tilemap, tiletexfile, layerNodes
void levelPlaceBox(sf::Vector2f pos, b32 isStatic, b2World* world,
                   std::map<u32, Tile*>& tilemap,
                   std::array<Entity*, LAYER_COUNT>& m_layerNodes,
                   const sf::Vector2u maxMapSize, const sf::Texture& tile_sheet,
                   Player* player);

u32 levelTileIDfromCoords(const u32 x, const u32 y,
                          const sf::Vector2u MaxMapSize);
