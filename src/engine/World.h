//
// Created by matjam on 12/17/2021.
//

#pragma once

#include <common.h>
#include "TileMap.h"

class World {
private:
    entt::registry m_registry{};
    sf::Vector2i playerLocation{};
    float scaleFactor = 0;
    TileMap m_map;

public:
    World(unsigned int width, unsigned int height) : m_map(sf::Vector2u{width, height}) {};

    World(const World &) = delete; // disable copying
    World &operator=(World const &) = delete; // disable assignment

    void load();

    void render(sf::RenderWindow &window);

    void update();
};


