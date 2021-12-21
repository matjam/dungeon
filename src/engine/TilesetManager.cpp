//
// Created by matjam on 12/19/2021.
//

#include "TilesetManager.h"

#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

#define TILE_SPAN 32 // tiles per row
#define ZIP_PASSWORD "X!u/+eDU+?b9s%rA"

using json = nlohmann::json;

Tileset &TilesetManager::getTileset(const std::string &tilesetName) {
    return m_tilesets.at(tilesetName);
}

sf::Texture &TilesetManager::getTexture(const std::string &tilesetName) {
    return m_textures[m_tilesets.at(tilesetName).textureIndex];
}

std::map<std::string, sf::Vector2f> &TilesetManager::getTilePositions(const std::string &name) {
    return m_tilesets.at(name).tilePositions;
}

// Will open a manifest file and then load a TileSet based on the configuration in that file. Should be faster than
// trying to generate the image using lots of little files and we can finesse the names of things.
bool TilesetManager::loadManifest(const std::string &manifestPath) {
    json j;
    std::ifstream input(manifestPath, std::ios::binary);
    if (!input.is_open()) {
        SPDLOG_CRITICAL("unable to open manifest {}", manifestPath);
        return false;
    }
    input >> j;

    // load the image used for all the tiles in this tileset
    m_images.emplace_back();
    sf::Image &image = m_images.back();
    if (!image.loadFromFile(j["file"])) {
        SPDLOG_CRITICAL("unable to load image {} in manifest {}", j["file"], manifestPath);
        return false;
    }

    // create a texture from the image.
    m_textures.emplace_back();
    sf::Texture &texture = m_textures.back();
    texture.create(image.getSize().x, image.getSize().y);
    texture.loadFromImage(image);

    // load the definitions of each tile into the tileset
    for (auto &[setName, setConfig]: j["sets"].items()) {
        sf::Vector2u tileSize{setConfig["size"]["width"], setConfig["size"]["height"]};

        m_tilesets[setName] = Tileset();
        auto &tileSet = m_tilesets.at(setName);

        tileSet.tileSize = tileSize;
        tileSet.textureIndex = m_textures.size() - 1;

        // iterate through the tiles listed in the JSON and set their positions.
        for (auto &[tileName, tile]: setConfig["tiles"].items()) {
            tileSet.tilePositions[tileName] = sf::Vector2f{tile["x"], tile["y"]};
            tileSet.tileCount++;
        }

        SPDLOG_INFO("loaded tileset {} with {} tiles", setName, tileSet.tileCount);
    }
    return true;
}
