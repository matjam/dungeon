//
// Created by matjam on 12/20/2021.
//

#pragma once

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>


class TilesetGenerator {
private:

public:
    TilesetGenerator() = default;

    TilesetGenerator(TilesetGenerator const &) = delete; // disable copying
    TilesetGenerator &operator=(TilesetGenerator const &) = delete; // disable assignment

    void run();
};


