//
// Created by matjam on 12/17/2021.
//

#pragma once

#include "common.h"
#include <map>
#include "TilesetManager.h"

class Sprites {
private:
    TilesetManager &m_tilesetManager = TilesetManager::get();


public:
    Sprites() = default;

    Sprites(const Sprites &) = delete; // disable copying
    Sprites &operator=(Sprites const &) = delete; // disable assignment
};


