cmake_minimum_required(VERSION 3.21)

project(dungeon)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
file(
        GLOB_RECURSE SOURCE_FILES
        CONFIGURE_DEPENDS  # Automatically reconfigure if source files are added/removed.
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/*.h
)

set(IMGUI_SOURCES imgui/imgui.cpp imgui/imgui_widgets.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_demo.cpp)

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Boost 1.77 REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${IMGUI_SOURCES})

# Enable warnings recommended for new projects.
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

# Ensure the C++17 standard is available.
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} PUBLIC src src/**)
target_link_libraries(${PROJECT_NAME} PRIVATE
        sfml-system
        sfml-network
        sfml-graphics
        sfml-window
        EnTT::EnTT
        spdlog::spdlog spdlog::spdlog_header_only
        OpenGL32)
