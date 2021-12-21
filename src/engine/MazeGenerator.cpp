//
// Created by matjam on 12/20/2021.
//

#include "MazeGenerator.h"

void MazeGenerator::create(sf::Uint32 width, sf::Uint32 height, sf::Uint64 seed) {
    m_width = width;
    m_height = height;
    m_rng = XoshiroCpp::Xoshiro256PlusPlus(seed);
}

bool MazeGenerator::generate() {
    return false;
}
