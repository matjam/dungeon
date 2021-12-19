//
// Created by matjam on 12/17/2021.
//

#include "World.h"

void World::render(sf::RenderWindow &window) {
    window.draw(m_map);
}

void World::update() {

}

void World::load() {
    // load this from JSON config eventually
    m_map.setTileConfig(
            {
                    {"WALL",            6},
                    {"WALL_BOTTOM",     0},
                    {"FLOOR",           64},
                    {"FLOOR_VARIANT_0", 64},
                    {"FLOOR_VARIANT_1", 65},
                    {"FLOOR_VARIANT_2", 66},
                    {"FLOOR_VARIANT_3", 67},
                    {"FLOOR_VARIANT_4", 68},
                    {"FLOOR_VARIANT_5", 69},
                    {"DOOR",            160}
            }
    );

    if (!m_map.load("data/tiny_dungeon_world.png", sf::Vector2u(16, 16))) {
        SPDLOG_CRITICAL("wasn't able to load world map");
    }

    m_map.setTileType(sf::Vector2i{1, 1}, Tile::Type::WALL);
    m_map.setScale(4.0f, 4.0f);
    m_map.update();
}
