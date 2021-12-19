//
// Created by matjam on 12/17/2021.
//

#pragma once

#include "common.h"
#include <map>

class Sprites {
private:
    std::map<std::string, sf::Texture> m_textures{};
public:
    Sprites() = default;

    Sprites(const Sprites &) = delete; // disable copying
    Sprites &operator=(Sprites const &) = delete; // disable assignment

    void loadSheet(const std::string &name, const std::string &filename);
};


