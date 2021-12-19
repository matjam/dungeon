//
// Created by matjam on 12/17/2021.
//

#pragma once

#include "common.h"

class PositionComponent {
private:
    sf::Vector2i m_position;

public:
    PositionComponent() = default;

    PositionComponent(const PositionComponent &) = delete; // disable copying
    PositionComponent &operator=(PositionComponent const &) = delete; // disable assignment

    void setPosition(sf::Vector2i position) {
        m_position = position;
    }

    sf::Vector2i getPosition() {
        return m_position;
    }
};


