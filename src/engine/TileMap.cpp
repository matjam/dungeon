//
// Created by matjam on 12/17/2021.
//

#include "TileMap.h"
#include <utility>
#include <fmt/format.h>

void TileMap::updateTileNames(Tile &tile, sf::Vector2i position) {
    switch (tile.type) {
        case Tile::Type::WALL: {
            if (getTileType(sf::Vector2i{position.x, position.y + 1}) == Tile::Type::WALL) {
                tile.baseTileName = "wall_stone_v_a";
            } else {
                tile.baseTileName = "wall_stone_h_a";
            }
            return;
        }
        case Tile::Type::DOOR: // doors are drawn as entities as they can be open or closed
        case Tile::Type::FLOOR: {
            tile.baseTileName = "floor_cobble_a";
            return;
        }
        case Tile::Type::STAIR_DOWN: {
            tile.baseTileName = "wall_stone_stair_down";
            return;
        }
        case Tile::Type::STAIR_UP: {
            tile.baseTileName = "wall_stone_stair_up";
            return;
        }
    }
}

void TileMap::update() {
    Tileset &ts = TilesetManager::get().getTileset("world");

    // populate the vertex array, with one quad per tile
    for (sf::Uint32 i = 0; i < (sf::Uint32) m_width; ++i) {
        for (sf::Uint32 j = 0; j < (sf::Uint32) m_height; ++j) {
            // get the current tile
            Tile tile = m_tiles.at(i + j * m_width);

            // update the tileNames for this tile
            updateTileNames(tile, sf::Vector2i{static_cast<int>(i), static_cast<int>(j)});

            // update the layers with the correct tile
            updateVertex(m_baseLayer, sf::Vector2u{i, j}, ts, tile.baseTileName);
        }
    }
}

void TileMap::updateVertex(sf::VertexArray &va, sf::Vector2u pos, Tileset &ts, std::string &tileName) const {
    // get a pointer to the current tile's quad
    sf::Vertex *quad = &va[(pos.x + pos.y * m_width) * 4];

    // define its 4 corners
    quad[0].position = sf::Vector2f((float) (pos.x * ts.tileSize.x), (float) (pos.y * ts.tileSize.y));
    quad[1].position = sf::Vector2f((float) ((pos.x + 1) * ts.tileSize.x), (float) (pos.y * ts.tileSize.y));
    quad[2].position = sf::Vector2f((float) ((pos.x + 1) * ts.tileSize.x), (float) ((pos.y + 1) * ts.tileSize.y));
    quad[3].position = sf::Vector2f((float) (pos.x * ts.tileSize.x), (float) ((pos.y + 1) * ts.tileSize.y));

    sf::Vector2f &tOff = ts.tilePositions[tileName];

    // define its 4 texture coordinates
    quad[0].texCoords = sf::Vector2f(tOff.x, tOff.y);
    quad[1].texCoords = sf::Vector2f(tOff.x + (float) ts.tileSize.x, tOff.y);
    quad[2].texCoords = sf::Vector2f(tOff.x + (float) ts.tileSize.x, tOff.y + (float) ts.tileSize.y);
    quad[3].texCoords = sf::Vector2f(tOff.x, tOff.y + (float) ts.tileSize.y);
}

void TileMap::draw(sf::RenderTarget &target, sf::RenderStates states) const {
    // apply the transform
    states.transform *= getTransform();

    // apply the tileset texture
    states.texture = &TilesetManager::get().getTexture("world");

    // draw the vertex array
    target.draw(m_baseLayer, states);

    // draw the next layer
    target.draw(m_topLayer, states);
}

bool TileMap::create(sf::Vector2u mapSize) {
    TilesetManager::get().loadManifest("data/tileset_manifest.json");

    m_tiles = std::vector<Tile>(mapSize.x * mapSize.y);
    m_width = mapSize.x;
    m_height = mapSize.y;

    // resize the vertex array to fit the level size
    m_baseLayer.setPrimitiveType(sf::Quads);
    m_baseLayer.resize(m_width * m_height * 4);

    // resize the vertex array to fit the level size
    m_topLayer.setPrimitiveType(sf::Quads);
    m_topLayer.resize(m_width * m_height * 4);

    // update the vertex array so the tileset will render.
    update();

    SPDLOG_INFO("tileset loaded");
    return true;
}


void TileMap::setTile(sf::Vector2i position, Tile &tile) {
    if (position.x < 0 || position.y < 0 || position.x >= static_cast<int>(m_width) ||
        position.y >= static_cast<int>(m_height)) {
        return;
    }

    sf::Uint32 index = position.x + position.y * m_width;
    m_tiles[index] = std::move(tile);
}

void TileMap::setTileType(sf::Vector2i position, Tile::Type t) {
    if (position.x < 0 || position.y < 0 || position.x >= static_cast<int>(m_width) ||
        position.y >= static_cast<int>(m_height)) {
        return;
    }

    sf::Uint32 index = position.x + position.y * m_width;
    m_tiles.at(index).type = t;
}

Tile &TileMap::getTile(sf::Vector2i position) {
    if (position.x < 0 || position.y < 0 || position.x >= static_cast<int>(m_width) ||
        position.y >= static_cast<int>(m_height)) {
        return m_outOfBoundsTile;
    }
    sf::Uint32 index = position.x + position.y * m_width;
    return m_tiles.at(index);
}

Tile::Type TileMap::getTileType(sf::Vector2i position) {
    if (position.x < 0 || position.y < 0 || position.x >= static_cast<int>(m_width) ||
        position.y >= static_cast<int>(m_height)) {
        return Tile::Type::WALL;
    }
    sf::Uint32 index = position.x + position.y * m_width;
    return m_tiles.at(index).type;
}