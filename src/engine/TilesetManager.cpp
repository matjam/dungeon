//
// Created by matjam on 12/19/2021.
//

#include "TilesetManager.h"

#include <string>
#include <nlohmann/json.hpp>
#include <cmrc/cmrc.hpp>

CMRC_DECLARE(dungeon);

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

// Will open a manifest file and then create a TileSet based on the configuration in that file. Should be faster than
// trying to generate the image using lots of little files and we can finesse the names of things.
bool TilesetManager::loadManifest(const std::string &manifestPath) {
    auto fs = cmrc::dungeon::get_filesystem();

    auto manifestData = fs.open(manifestPath);
    auto manifest = std::string(manifestData.begin(), manifestData.end());
    json j = json::parse(manifest);

    // create the image used for all the tiles in this tileset
    m_images.emplace_back();
    sf::Image &image = m_images.back();

    auto imageData = fs.open(j["file"]);
    if (!image.loadFromMemory(imageData.begin(), imageData.size())) {
        SPDLOG_CRITICAL("unable to create image {} in manifest {}", j["file"], manifestPath);
        return false;
    }

    // create a texture from the image.
    m_textures.emplace_back();
    sf::Texture &texture = m_textures.back();
    texture.create(image.getSize().x, image.getSize().y);
    texture.loadFromImage(image);

    // create the definitions of each tile into the tileset
    for (auto &[setName, setConfig]: j["sets"].items()) {
        sf::Vector2u tileSize{setConfig["size"]["width"], setConfig["size"]["height"]};

        m_tilesets[setName] = Tileset();
        auto &tileSet = m_tilesets.at(setName);

        tileSet.tileSize = tileSize;
        tileSet.textureIndex = (unsigned int) m_textures.size() - 1;

        // iterate through the tiles listed in the JSON and set their positions.
        for (auto &[tileName, tile]: setConfig["tiles"].items()) {
            tileSet.tilePositions[tileName] = sf::Vector2f{tile["x"], tile["y"]};
            tileSet.tileCount++;
        }

        SPDLOG_INFO("loaded tileset {} with {} tiles", setName, tileSet.tileCount);
    }
    return true;
}
