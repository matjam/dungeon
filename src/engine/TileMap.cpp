//
// Created by matjam on 12/17/2021.
//

#include "TileMap.h"
#include <filesystem>

void TileMap::update() {
    // populate the vertex array, with one quad per tile
    for (sf::Uint32 i = 0; i < (sf::Uint32) m_width; ++i) {
        for (sf::Uint32 j = 0; j < (sf::Uint32) m_height; ++j) {
            // get the current tile
            Tile tile = m_tiles.at(i + j * m_width);

            // find its position in the tileset texture
            sf::Uint32 tu = tile.textureIndex % (m_tileset.getSize().x / m_tileSize.x);
            sf::Uint32 tv = tile.textureIndex / (m_tileset.getSize().x / m_tileSize.x);

            // get a pointer to the current tile's quad
            sf::Vertex *quad = &m_vertices[(i + j * m_width) * 4];

            // define its 4 corners
            quad[0].position = sf::Vector2f((float) (i * m_tileSize.x), (float) (j * m_tileSize.y));
            quad[1].position = sf::Vector2f((float) ((i + 1) * m_tileSize.x), (float) (j * m_tileSize.y));
            quad[2].position = sf::Vector2f((float) ((i + 1) * m_tileSize.x), (float) ((j + 1) * m_tileSize.y));
            quad[3].position = sf::Vector2f((float) (i * m_tileSize.x), (float) ((j + 1) * m_tileSize.y));

            // define its 4 texture coordinates
            quad[0].texCoords = sf::Vector2f((float) (tu * m_tileSize.x), (float) (tv * m_tileSize.y));
            quad[1].texCoords = sf::Vector2f((float) ((tu + 1) * m_tileSize.x), (float) (tv * m_tileSize.y));
            quad[2].texCoords = sf::Vector2f((float) ((tu + 1) * m_tileSize.x), (float) ((tv + 1) * m_tileSize.y));
            quad[3].texCoords = sf::Vector2f((float) (tu * m_tileSize.x), (float) ((tv + 1) * m_tileSize.y));
        }
    }
}

void TileMap::draw(sf::RenderTarget &target, sf::RenderStates states) const {
    // apply the transform
    states.transform *= getTransform();

    // apply the tileset texture
    states.texture = &m_tileset;

    // draw the vertex array
    target.draw(m_vertices, states);
}

bool TileMap::load(const std::string &tileset, sf::Vector2u tileSize) {
    if (m_tileConfig.empty()) {
        SPDLOG_CRITICAL("m_tileConfig is empty, cannot load tileset");
        return false;
    }

    SPDLOG_INFO("loading tileset {}", tileset);

    // make a texture of a decent size


    // load the tileset texture
    if (!m_tileset.loadFromFile(tileset)) {
        SPDLOG_CRITICAL("loadFromFile failed, cannot load tileset");
        return false;
    }

    // resize the vertex array to fit the level size
    m_vertices.setPrimitiveType(sf::Quads);
    m_vertices.resize(m_width * m_height * 4);

    // store the size of the tiles
    m_tileSize = tileSize;

    // update the out of bounds tile
    m_outOfBoundsTile.textureIndex = m_tileConfig["WALL"];

    // make sure that every Tile has a tileTextureIndex set for it.
    refreshTileTextureIndexes();

    // update the vertex array so the tileset will render.
    update();

    SPDLOG_INFO("tileset loaded");
    return true;
}


void TileMap::setTile(sf::Vector2i position, Tile tile) {
    if (position.x < 0 || position.y < 0 || position.x >= m_width || position.y >= m_height) {
        SPDLOG_WARN("out of bounds access position {},{}", position.x, position.y);
        return;
    }

    sf::Uint32 index = position.x + position.y * m_width;
    Tile &t = m_tiles.at(index);
    t = tile;
}

void TileMap::setTileType(sf::Vector2i position, Tile::Type t) {
    if (position.x < 0 || position.y < 0 || position.x >= m_width || position.y >= m_height) {
        SPDLOG_WARN("out of bounds access position {},{}", position.x, position.y);
        return;
    }

    sf::Uint32 index = position.x + position.y * m_width;
    Tile &tile = m_tiles.at(index);
    tile.type = t;
    generateTextureIndexForTile(tile);
}

void TileMap::generateTextureIndexForTile(Tile &tile) {
    if (m_tileConfig.empty()) {
        SPDLOG_CRITICAL("m_tileConfig is empty");
        return;
    }

    // set the rendered tile index
    switch (tile.type) {
        case Tile::Type::WALL:
            tile.textureIndex = m_tileConfig["WALL"];
            break;
        default:
            tile.textureIndex = m_tileConfig["FLOOR_VARIANT_0"];
    }
}

// if you change the tileConfig then you'll need to refresh the texture indexes for all tiles
void TileMap::refreshTileTextureIndexes() {
    for (auto &tile: m_tiles) {
        generateTextureIndexForTile(tile);
    }
}

Tile &TileMap::getTile(sf::Vector2i position) {
    if (position.x < 0 || position.y < 0 || position.x >= m_width || position.y >= m_height) {
        SPDLOG_WARN("out of bounds access position {},{}", position.x, position.y);
        return m_outOfBoundsTile;
    }
    sf::Uint32 index = position.x + position.y * m_width;
    return m_tiles.at(index);
}
