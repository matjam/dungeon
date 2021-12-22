//
// Created by matjam on 12/20/2021.
//

#include "MazeGenerator.h"

void MazeGenerator::create(sf::Vector2u size, sf::Uint64 seed) {
    m_width = size.x;
    m_height = size.y;
    m_rng = XoshiroCpp::Xoshiro256PlusPlus(seed);
    m_map.create(size);
}

bool MazeGenerator::generate() {
    placeRooms();


    return true;
}

void MazeGenerator::placeRooms() {
    
}

void MazeGenerator::placeHallways() {

}

void MazeGenerator::placeConnectors() {

}

void MazeGenerator::removeDeadEnds() {

}
