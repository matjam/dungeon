cmake_minimum_required(VERSION 3.21)

project(dungeon)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
file(
        GLOB_RECURSE SOURCE_FILES
        CONFIGURE_DEPENDS  # Automatically reconfigure if source files are added/removed.
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/*.h
)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

file(DOWNLOAD "https://raw.githubusercontent.com/vector-of-bool/cmrc/master/CMakeRC.cmake"
        "${CMAKE_BINARY_DIR}/CMakeRC.cmake")
include("${CMAKE_BINARY_DIR}/CMakeRC.cmake")

cmrc_add_resource_library(
        dungeon-resources

        ALIAS dungeon::rc
        NAMESPACE dungeon

        data/tileset_manifest.json
        data/tileset.png
)

# spdlog uses fmt, but I also want to use fmt.
add_definitions(-DSPDLOG_FMT_EXTERNAL)

# Initialize vcpkg dependencies
find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Boost 1.77 REQUIRED)
find_package(GLEW REQUIRED)
find_package(libtcod CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(libzippp CONFIG REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Enable warnings recommended for new projects.
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif ()

# Ensure the C++17 standard is available.
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME} PUBLIC src src/** ./)

target_link_libraries(${PROJECT_NAME} PRIVATE dungeon::rc)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-system)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-network)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-window)
target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)
target_link_libraries(${PROJECT_NAME} PRIVATE libtcod::libtcod)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE libzip::zip libzippp::libzippp)


add_executable(tileset_generator tools/main.cpp tools/TilesetGenerator.cpp)
target_include_directories(tileset_generator PUBLIC tools/**)
target_link_libraries(tileset_generator PRIVATE
        sfml-system
        sfml-network
        sfml-graphics
        sfml-window
        spdlog::spdlog_header_only
        nlohmann_json nlohmann_json::nlohmann_json
        libzip::zip libzippp::libzippp)
