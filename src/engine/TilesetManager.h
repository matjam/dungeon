//
// Created by matjam on 12/19/2021.
//

#pragma once

#include "common.h"
#include <utility>
#include <nlohmann/json.hpp>

struct Tileset {
    unsigned int textureIndex; // index of the texture this tileset uses.
    sf::Vector2u tileSize;
    sf::Uint32 tileCount{0};
    std::map<std::string, sf::Vector2f> tilePositions;
};

class TilesetManager {
public:
    // singleton pattern
    static TilesetManager &get() {
        static TilesetManager tm;
        return tm;
    }

    TilesetManager(TilesetManager const &) = delete; // disable copying
    TilesetManager &operator=(TilesetManager const &) = delete; // disable assignment

    // create a Tileset containing multiple Tile sets using a manifest JSON.
    bool loadManifest(const std::string &manifestPath);

    sf::Texture &getTexture(const std::string &tilesetName);

    std::map<std::string, sf::Vector2f> &getTilePositions(const std::string &tilesetName);

    Tileset &getTileset(const std::string &tilesetName);

private:
    std::map<std::string, Tileset> m_tilesets;

    std::vector<sf::Image> m_images;
    std::vector<sf::Texture> m_textures;

    TilesetManager() = default; // private constructor so only the singleton get() method can construct the class.
};


