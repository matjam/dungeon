//
// Created by matjam on 12/17/2021.
//

#pragma once

#include <utility>

#include "common.h"


struct Tile {
    enum Type : unsigned int {
        FLOOR,
        WALL,
        DOOR
    };

    Type type{}; // what kind of tile this is
    int textureIndex{-1}; // which tile to use for rendering; we default to -1 to indicate it's unset
};

class TileMap : public sf::Drawable, public sf::Transformable {
private:
    sf::Texture m_tileset;
    sf::VertexArray m_vertices;
    sf::Int32 m_width;
    sf::Int32 m_height;
    sf::Vector2u m_tileSize;
    std::vector<Tile> m_tiles;
    std::map<std::string, int> m_tileConfig{}; // this should be a map to a vector so we can handle variants
    Tile m_outOfBoundsTile{Tile::Type::WALL, -1};

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

public:
    TileMap() = delete;

    TileMap(const TileMap &) = delete; // disable copying
    TileMap &operator=(TileMap const &) = delete; // disable assignment

    explicit TileMap(sf::Vector2u mapSize) : m_tiles(mapSize.x * mapSize.y), m_width{(sf::Int32) mapSize.x},
                                             m_height{(sf::Int32) mapSize.y} {}

    // before you can load a tileset, you must call setTileConfig() with a map that defines what different tile types
    // map to in the tileset you're loading.
    void setTileConfig(std::map<std::string, int> tileConfig) {
        m_tileConfig = std::move(tileConfig);
    }

    bool load(const std::string &tileset, sf::Vector2u tileSize);

    // update() only needs to be called when you've finished making changes to the TileMap, it should not need to be
    // called every frame.
    void update();

    void setTile(sf::Vector2i position, Tile tile);

    void setTileType(sf::Vector2i position, Tile::Type t);

    Tile &getTile(sf::Vector2i position);

    // given a specific Tile, sets the equivalent textureIndex in the tile.
    void generateTextureIndexForTile(Tile &tile);

    // Refreshes texture indexes for all tiles in the map.
    void refreshTileTextureIndexes();
};


