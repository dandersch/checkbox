#pragma once
#include "pch.h"

template<typename Resource>
class ResourcePool;
class Player;
class Tile;
class Entity;

static const u32 tile_width = 64;
static const u32 tile_height = 64;

enum Layer
{
    LAYER_BACK,
    LAYER_MID,
    LAYER_FORE,
    LAYER_COUNT
};

void levelBuild(std::map<u32, Tile*>& tilemap, std::map<u32, Tile*>& tilemap_bg,
                b2World* world, ResourcePool<sf::Texture>& textures,
                ResourcePool<sf::Image>& levels, sf::Vector2u& maxMapSize,
                std::array<Entity*, LAYER_COUNT>& m_layerNodes, Player* player,
                const std::string& levelName_mid = "",
                const std::string& levelName_back = "",
                const std::string& tileSheet = "legacy_dungeon.png");

// tilemap, tiletexfile, layerNodes
void levelPlaceBox(sf::Vector2f pos, b32 isStatic, b2World* world,
                   std::map<u32, Tile*>& tilemap,
                   std::array<Entity*, LAYER_COUNT>& m_layerNodes,
                   const sf::Vector2u maxMapSize, const sf::Texture& tile_sheet,
                   Player* player);

u32 levelTileIDfromCoords(const u32 x, const u32 y,
                          const sf::Vector2u MaxMapSize);

b2Body* createBox(b2World* world, i32 posX, i32 posY, i32 sizeX, i32 sizeY,
                  b2BodyType type, void* userData, Player* player,
                  b32 collidable = true);
