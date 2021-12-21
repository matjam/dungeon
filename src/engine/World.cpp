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
    m_map.setScale(5.0f, 5.0f);
    m_map.update();
}