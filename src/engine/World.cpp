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
    m_map.create(sf::Vector2u{120, 120});
    m_map.setScale(4.0f, 4.0f);

    m_maze.setConfig(MazeConfig{
            1337,
            120,
            120,
            3,
            9,
            1000,
            50000,
    });
    m_maze.generate();

    for (auto y = 0; y < 120; y++) {
        for (auto x = 0; x < 120; x++) {
            auto t = m_maze.getTile(x, y);
            switch (t.type) {
                case HALL:
                case ROOM:
                    m_map.setTileType(sf::Vector2i(x, y), Tile::Type::FLOOR);
                    break;
                case DOOR:
                    m_map.setTileType(sf::Vector2i(x, y), Tile::Type::DOOR);
                    break;
                case WALL:
                    m_map.setTileType(sf::Vector2i(x, y), Tile::Type::WALL);
                    break;
                default:
                    SPDLOG_WARN("maze tile type at {},{} is not recognized", x, y);
            }
        }
    }

    m_map.update();
}
