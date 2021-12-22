//
// Created by matjam on 12/20/2021.
//

#pragma once

#include "common.h"
#include "TileMap.h"
#include "util/XoshiroCpp.h"
#include <random>

class MazeGenerator {
private:
    XoshiroCpp::Xoshiro256PlusPlus m_rng;
    sf::Uint32 m_width = 0;
    sf::Uint32 m_height = 0;
    TileMap m_map;

    sf::Uint32 randomInt(sf::Uint32 min, sf::Uint32 max) {
        std::uniform_int_distribution<sf::Uint32> dist(min, max);
        return dist(m_rng);
    }

    void placeRooms();

    void placeHallways();

    void placeConnectors();

    void removeDeadEnds();

public:
    MazeGenerator() = default;

    MazeGenerator(MazeGenerator const &) = delete; // disable copying
    MazeGenerator &operator=(MazeGenerator const &) = delete; // disable assignment

    void create(sf::Vector2u size, sf::Uint64 seed);

    bool generate();
};


