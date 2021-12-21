#pragma once

#include "common.h"
#include <random>

struct MazeConfig {
    int seed;
    int w;
    int h;
    int room_min;
    int room_max;
    int room_attempts;
    int extra_door_chance;
    int culling;
    int sleep;
};

typedef struct MazeConfig MazeConfig;

enum MazeTileType : char {
    WALL,
    HALL,
    ROOM,
    DOOR,
    CONNECTOR,
    INVALID,
};

struct MazeTile {
    enum MazeTileType type;
    int x;
    int y;
    int w;
    int h;
    int region_id;
};

typedef struct MazeTile MazeTile;

class Maze {
public:
    Maze() = default;

    Maze(MazeConfig *);

    ~Maze();

    void setConfig(MazeConfig *);

    void generate();

    void render(sf::RenderTarget *);

    bool shouldRender();

    sf::Mutex renderMutex;
    std::vector<std::shared_ptr<sf::RectangleShape>> tile_shapes;

    void setTile(MazeTile);

    MazeTile getTile(int x, int y);

    MazeTile getAtDirection(int x, int y, char face, int distance);

    void carveToDirection(int *x, int *y, char face, int distance, MazeTile);

    void placeRooms();

    void placeHallways();

    void placeConnectors();

    void removeDeadEnds();

    std::vector<MazeTile> findDeadEnds();

    void connectRegions();

    bool scanForWalls();

    void startWalking(int x, int y);

    bool roomExists(int, int, int, int);

    bool getConnectorRegions(int x, int y, int *r1, int *r2);

    void updateRegion(int old_region, int new_region);

    bool isUnconnected(int);

    bool isDeadEnd(int, int);

    bool isNextToDoor(int, int);

    int findConnectedRegion(int);

    int getRandom(int start, int end);

    sf::Color getRandomColor();

    bool mazeWalk(int *x, int *y);

    bool mazeHunt(int *x, int *y);

    std::array<char, 4> shuffleDirections();

private:
    struct MazeConfig config;
    MazeTile *tile_map;
    int root_region;
    int current_region;
    bool dirty;
    std::unique_ptr<std::default_random_engine> rand_engine;
    std::map<int, sf::Color> region_colors;
    std::set<int> unconnected_regions;
    size_t total_regions;

    std::map<int, int> connections;                // old region to new region id
    std::map<int, std::vector<MazeTile>> region_tiles;      // all tiles for a region
    std::map<int, std::vector<MazeTile>> region_connectors; // all connectors for a region
    std::vector<MazeTile> connectors;                  // all connectors that exist
};
