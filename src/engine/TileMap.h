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
    enum class Type : sf::Uint8 {
        FLOOR,
        WALL,
        DOOR,
        STAIR_DOWN,
        STAIR_UP,
    };

    Tile::Type type{Type::WALL};

    // This is written in the update() pass to map the specific tile from the tilemap to the vertex array that renders.
    std::string baseTileName{"wall_stone_v_a"}; // name of the tile in the tileset to use when rendering
};

class TileMap : public sf::Drawable, public sf::Transformable {
private:
    sf::VertexArray m_baseLayer;
    sf::VertexArray m_topLayer;
    sf::Uint32 m_width;
    sf::Uint32 m_height;
    std::vector<Tile> m_tiles;
    Tile m_outOfBoundsTile{};

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

    void updateVertex(sf::VertexArray &va, sf::Vector2u pos, Tileset &ts, std::string &tileName) const;

    void updateTileNames(Tile &tile, sf::Vector2i position);

public:
    TileMap() = default;

    TileMap(const TileMap &) = delete; // disable copying
    TileMap &operator=(TileMap const &) = delete; // disable assignment

    bool create(sf::Vector2u mapSize);

    // update() only needs to be called when you've finished making changes to the TileMap, it should not need to be
    // called every frame. It will scan through all of the tiles and configure the tileEntityName to the correct type
    // based on the tilemap configuration.
    void update();

    void setTile(sf::Vector2i position, Tile &tile);

    void setTileType(sf::Vector2i position, Tile::Type t);

    Tile &getTile(sf::Vector2i position);

    Tile::Type getTileType(sf::Vector2i position);

};


