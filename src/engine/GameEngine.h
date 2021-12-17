//
// Created by matjam on 12/16/2021.
//

#pragma once

#include "common.h"

class GameEngine {
private:

public:
    GameEngine();

    GameEngine(const GameEngine &) = delete; // disable copying
    GameEngine &operator=(GameEngine const &) = delete; // disable assignment
    int run(int argc, char **argv);
};


