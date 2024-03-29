//
// Created by matjam on 12/16/2021.
//

#pragma once

#include "common.h"
#include "World.h"

class GameEngine {
private:
    bool m_running{true};
    sf::Int64 m_renderTime{};
    sf::Mutex m_renderMutex;
    sf::RenderWindow m_window;
    std::shared_ptr<spdlog::logger> m_console;
    sf::Font m_font;
    sf::Text m_fpsDisplayText;
    World m_world;

public:
    GameEngine();

    GameEngine(const GameEngine &) = delete; // disable copying
    GameEngine &operator=(GameEngine const &) = delete; // disable assignment
    int run();

    void renderThread();

    void updateThread();

    void eventHandlerThread();

    void renderTimeThread() const;
};


