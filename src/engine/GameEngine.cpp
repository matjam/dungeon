//
// Created by matjam on 12/16/2021.
//

#include "GameEngine.h"
#include <stdlib.h>
#include "util/Util.h"


GameEngine::GameEngine() : m_window(sf::VideoMode(1920, 1080), "Game") {
    m_window.setVerticalSyncEnabled(true);
//    ImGui::SFML::Init(m_window);

    m_console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(m_console);
//    spdlog::set_pattern("[%^%8l%$] [%s/%!:%#] %v");
    m_console->set_level(spdlog::level::trace);

    SPDLOG_INFO("intializing GameEngine");

    m_basePath = getEnv("BASEPATH");
    if (!m_basePath.empty()) {
        SPDLOG_INFO("BASEPATH set: {}", m_basePath);
    }

    if (!m_font.loadFromFile("data/square.ttf")) {
        SPDLOG_CRITICAL("unable to load game font file");
    }

    m_fpsDisplayText.setFont(m_font);
    m_fpsDisplayText.setCharacterSize(16);
    m_fpsDisplayText.setFillColor(sf::Color::Yellow);
}

void GameEngine::renderThread() {
    SPDLOG_INFO("renderThread starting");

    m_window.setActive(true);

    sf::Clock deltaClock;
    sf::Clock clock;
    sf::Clock renderTime;
    while (m_running) {
        m_window.clear();
        m_renderMutex.lock();

        renderTime.restart();
        m_world.render(m_window);
        m_renderTime = renderTime.restart().asMicroseconds();

        if (m_fpsDisplayEnabled) {
            float fpsTime = clock.restart().asSeconds();
            float fps = 1.f / fpsTime;
            m_fpsDisplayText.setString(fmt::format("FPS {}", fps));
            m_window.draw(m_fpsDisplayText);
        }

        m_renderMutex.unlock();
        m_window.display();
    }

    SPDLOG_INFO("renderThread exiting");
}

void GameEngine::updateThread() {
    SPDLOG_INFO("updateThread starting");

    while (m_running) {
        sf::sleep(sf::seconds(1));
        m_renderMutex.lock();
        m_world.update();
        m_renderMutex.unlock();
    }

    SPDLOG_INFO("updateThread exiting");
}

void GameEngine::eventHandlerThread() {
    SPDLOG_INFO("eventHandlerThread starting");
    sf::Event event{};
    while (m_running) {
        while (m_window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                SPDLOG_INFO("received closed event for main window");
                m_running = false;
            }
            if (event.type == sf::Event::Resized) {
                sf::FloatRect visibleArea(0, 0, (float) event.size.width, (float) event.size.height);
                m_window.setView(sf::View(visibleArea));
            }
        }
        sf::sleep(sf::milliseconds(50));
    }
    SPDLOG_INFO("eventHandlerThread exiting");
}

void GameEngine::renderTimeThread() {
    SPDLOG_INFO("renderTimeThread starting");

    while (m_running) {
        sf::sleep(sf::seconds(1));
        SPDLOG_INFO("frametime: {:0.1f}ms", (float) m_renderTime / 1000);
    }

    SPDLOG_INFO("renderTimeThread exiting");
}

int GameEngine::run() {
    SPDLOG_INFO("GameEngine starting");

    m_world.load();

    // launch the renderThread.
    sf::Thread thread1([&]() { renderThread(); });
    thread1.launch();

    // launch the updateThread.
    sf::Thread thread2([&]() { updateThread(); });
    thread2.launch();

    // launch the renderTimeThread
    sf::Thread thread3([&]() { renderTimeThread(); });
    thread3.launch();

    m_window.setActive(false);
    eventHandlerThread(); // it's not really a thread; it's running on the main thread
    thread3.wait();
    thread2.wait();
    thread1.wait();
    m_window.close();

    SPDLOG_INFO("GameEngine exiting");
    return EXIT_SUCCESS;
}