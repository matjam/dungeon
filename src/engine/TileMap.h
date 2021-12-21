//
// Created by matjam on 12/17/2021.
//

#pragma once

#include <utility>

#include "common.h"
#include "TilesetManager.h"

/*
 * This set of types and classes are implemented specifically to use the Tiny Dungeon tileset
 * https://www.oryxdesignlab.com/products/tiny-dungeon-tileset
 */

struct Tile {
    enum Type : unsigned int {
        FLOOR,
        WALL,
        DOOR,
        OTHER
    };

    enum Style : unsigned int {
        STONE,
        SEWER,
        CRYPT,
        CAVE
    };

    Type type{WALL};
    Style style{STONE};
    std::string other;

    // This is written in the update() pass to map the specific tile from the tilemap to the vertex array that renders.
    std::string baseTileName{"wall_stone_v_a"}; // name of the tile in the tileset to use when rendering
    std::string topTileName{"none"};
};

class TileMap : public sf::Drawable, public sf::Transformable {
private:
    sf::VertexArray m_baseLayer;
    sf::VertexArray m_topLayer;
    sf::Int32 m_width;
    sf::Int32 m_height;
    std::vector<Tile> m_tiles;
    Tile m_outOfBoundsTile{};

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    void updateVertex(sf::VertexArray &va, sf::Vector2u pos, Tileset &ts, std::string &tileName) const;

    void updateTileNames(Tile &tile, sf::Vector2u position);

public:
    TileMap() = delete;

    TileMap(const TileMap &) = delete; // disable copying
    TileMap &operator=(TileMap const &) = delete; // disable assignment

    explicit TileMap(sf::Vector2u mapSize) : m_tiles(mapSize.x * mapSize.y), m_width{(sf::Int32) mapSize.x},
                                             m_height{(sf::Int32) mapSize.y} {}

    bool load();

    // update() only needs to be called when you've finished making changes to the TileMap, it should not need to be
    // called every frame. It will scan through all of the tiles and configure the tileEntityName to the correct type
    // based on the tilemap configuration.
    void update();

    void setTile(sf::Vector2i position, Tile &tile);

    void setTileType(sf::Vector2i position, Tile::Type t);

    Tile &getTile(sf::Vector2i position);

};


