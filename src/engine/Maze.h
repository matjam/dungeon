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
    int culling;
};

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

class Maze {
public:
    Maze() = default;

    void setConfig(MazeConfig);

    void generate();

    MazeTile getTile(int x, int y);


private:
    MazeConfig m_config{};
    std::vector<MazeTile> m_tile_map;
    int m_root_region = 0;
    int m_current_region = 0;
    int m_total_regions = 0;

    std::unique_ptr<std::default_random_engine> m_rand_engine;
    std::set<int> m_unconnected_regions;

    std::map<int, int> m_connections;                // old region to new region id
    std::map<int, std::vector<MazeTile>> m_region_tiles;      // all tiles for a region
    std::map<int, std::vector<MazeTile>> m_region_connectors; // all connectors for a region
    std::vector<MazeTile> m_connectors;                  // all connectors that exist

    void setTile(MazeTile);


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

    bool isDeadEnd(int, int);

    bool isNextToDoor(int, int);

    int findConnectedRegion(int);

    int getRandom(int start, int end);

    bool mazeWalk(int *x, int *y);

    bool mazeHunt(int *x, int *y);

    std::array<char, 4> shuffleDirections();
};
