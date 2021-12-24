//
// Created by matjam on 12/17/2021.
//

#pragma once

#include <common.h>
#include "TileMap.h"
#include "TilesetManager.h"
#include "Maze.h"

class World {
private:
    entt::registry m_registry{};
    sf::Vector2i playerLocation{};
    TileMap m_map;
    Maze m_maze;

public:
    World() = default;

    World(const World &) = delete; // disable copying
    World &operator=(World const &) = delete; // disable assignment

    void load();

    void render(sf::RenderWindow &window);

    void update();
};


