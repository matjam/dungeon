//
// Created by matjam on 12/17/2021.
//

#include "Sprites.h"

void Sprites::loadSheet(const std::string &name, const std::string &filename) {
    sf::Texture texture;
    texture.setSmooth(false);
    if (!texture.loadFromFile(filename)) {
        SPDLOG_CRITICAL("unable to load {} from {}", name, filename);
        return;
    }
    m_textures[name] = texture;
}
