//
// Created by matjam on 12/20/2021.
//

#pragma once

#include "common.h"
#include "TileMap.h"


class MazeGenerator {
private:

public:
    MazeGenerator();

    MazeGenerator(MazeGenerator const &) = delete; // disable copying
    MazeGenerator &operator=(MazeGenerator const &) = delete; // disable assignment

    void create(sf::Uint32 width, sf::Uint32 height, sf::Uint32 seed);
};


