#include "Maze.h"

/*
 * TODO: rewrite
 */

using namespace std;


void Maze::setConfig(MazeConfig mazeConfig) {
    m_config = mazeConfig;

    m_tile_map.clear();
    m_tile_map.resize(m_config.w * m_config.h);

    m_rand_engine = make_unique<std::default_random_engine>(m_config.seed);

    m_total_regions = 0;
    m_current_region = 0;
    m_unconnected_regions.clear();
    m_region_tiles.clear();
    m_region_connectors.clear();
    m_connections.clear();
    m_connectors.clear();
}


void Maze::generate() {
    SPDLOG_INFO("generating with seed {}", m_config.seed);
    m_current_region = 1;

    SPDLOG_INFO("placing rooms");
    placeRooms();

    SPDLOG_INFO("placing hallways");
    placeHallways();

    auto found_walls = scanForWalls();
    while (found_walls) {
        found_walls = scanForWalls();
    }

    SPDLOG_INFO("placingm_connectors");
    placeConnectors();

    SPDLOG_INFO("connecting regions");
    connectRegions();

    SPDLOG_INFO("removing dead ends");
    removeDeadEnds();
}

bool Maze::isDeadEnd(int x, int y) {
    MazeTile n = getAtDirection(x, y, 'N', 1);
    MazeTile s = getAtDirection(x, y, 'S', 1);
    MazeTile w = getAtDirection(x, y, 'W', 1);
    MazeTile e = getAtDirection(x, y, 'E', 1);

    auto total_walls = (n.type == WALL) + (n.type == INVALID) +
                       (s.type == WALL) + (s.type == INVALID) +
                       (w.type == WALL) + (w.type == INVALID) +
                       (e.type == WALL) + (e.type == INVALID);

    if (total_walls > 2) {
        return true;
    }

    return false;
}

bool Maze::isNextToDoor(int x, int y) {
    MazeTile n = getAtDirection(x, y, 'N', 1);
    MazeTile s = getAtDirection(x, y, 'S', 1);
    MazeTile w = getAtDirection(x, y, 'W', 1);
    MazeTile e = getAtDirection(x, y, 'E', 1);
    MazeTile n2 = getAtDirection(x, y, 'N', 2);
    MazeTile s2 = getAtDirection(x, y, 'S', 2);
    MazeTile w2 = getAtDirection(x, y, 'W', 2);
    MazeTile e2 = getAtDirection(x, y, 'E', 2);

    auto total_doors = (n.type == DOOR) + (s.type == DOOR) + (w.type == DOOR) +
                       (e.type == DOOR) + (n2.type == DOOR) +
                       (s2.type == DOOR) + (w2.type == DOOR) +
                       (e2.type == DOOR);

    if (total_doors > 0) {
        return true;
    }

    return false;
}

vector<MazeTile> Maze::findDeadEnds() {
    vector<MazeTile> dead_ends;

    for (int y = 0; y < m_config.h; y += 1) {
        for (int x = 0; x < m_config.w; x += 1) {
            auto tile = getTile(x, y);
            if (tile.type == HALL || tile.type == DOOR) {
                if (isDeadEnd(x, y)) {
                    dead_ends.push_back(tile);
                }
            }
        }
    }
    shuffle(dead_ends.begin(), dead_ends.end(), *m_rand_engine);

    return dead_ends;
}

// scan for any tiles that are dead ends
void Maze::removeDeadEnds() {

    int removed = 0;

    while (removed < m_config.culling) {
        int previous_removed = removed;

        auto dead_ends = findDeadEnds();
        for_each(dead_ends.begin(), dead_ends.end(), [&](MazeTile t) {
            t = {WALL, t.x, t.y, 0, 0, 0};
            setTile(t);
            removed++;
        });
        if (previous_removed == removed) {
            break;
        }
    }

    MazeTile t = getTile(0, 0);
    if (t.type == WALL && t.region_id != 0) {
        SPDLOG_ERROR("rogue region_id {} in hall at 0,0", t.region_id);
    }

    SPDLOG_INFO("removed {} halls total", removed);
}

// fill any spots we missed
bool Maze::scanForWalls() {
    bool found_walls = false;
    for (auto x = 0; x < m_config.w; x += 2) {
        for (auto y = 0; y < m_config.h; y += 2) {
            if (getTile(x, y).type == WALL) {
                found_walls = true;
                startWalking(x, y);
            }
        }
    }

    return found_walls;
}

void Maze::placeHallways() {
    int x = 0;
    int y = 0;

    bool room_exists = true;
    while (room_exists) {
        x = getRandom(0, m_config.w / 2) * 2;
        y = getRandom(0, m_config.h / 2) * 2;
        if (getTile(x, y).type != ROOM)
            room_exists = false;
    }
    startWalking(x, y);
}

// build a list of all the possiblem_connectors.
void Maze::placeConnectors() {
    for (int x = 0; x < m_config.w; x++) {
        for (int y = 0; y < m_config.h; y++) {
            auto tile = getTile(x, y);

            // keep a map of all the regions and their tiles for later
            m_region_tiles[tile.region_id].push_back(tile);

            // only walls can becomem_connectors
            if (getTile(x, y).type != WALL)
                continue;

            // consider the tiles on either side of this tile, and see if they
            // are valid regions.

            int region1 = 0;
            int region2 = 0;

            if (getConnectorRegions(x, y, &region1, &region2)) {
                // This is a connector
                MazeTile connector_tile{CONNECTOR, x, y, 1, 1, 0};
                setTile(connector_tile);

                m_region_connectors[region1].push_back(connector_tile);
                m_region_connectors[region2].push_back(connector_tile);
                m_unconnected_regions.insert(region1);
                m_unconnected_regions.insert(region2);
                m_connectors.push_back(connector_tile);
            }
        }
    }

    SPDLOG_INFO("{} regions found withm_connectors",
                m_unconnected_regions.size());
    m_total_regions = m_unconnected_regions.size();
}

// gets the two regions connected by a given point, probably a connector
// or maybe a wall if we are in that phase. We only consider ROOMs and HALLs.
bool Maze::getConnectorRegions(int x, int y, int *r1, int *r2) {
    // check W and E
    auto e = getAtDirection(x, y, 'E', 1);
    auto w = getAtDirection(x, y, 'W', 1);

    if ((e.type == HALL && w.type == ROOM) ||
        (e.type == ROOM && w.type == HALL) ||
        (e.type == ROOM && w.type == ROOM)) {
        *r1 = w.region_id;
        *r2 = e.region_id;
        return true;
    }

    // check N and S
    auto n = getAtDirection(x, y, 'N', 1);
    auto s = getAtDirection(x, y, 'S', 1);

    if ((n.type == HALL && s.type == ROOM) ||
        (n.type == ROOM && s.type == HALL) ||
        (n.type == ROOM && s.type == ROOM)) {
        *r1 = n.region_id;
        *r2 = s.region_id;
        return true;
    }

    return false;
}

void Maze::startWalking(int x, int y) {
    setTile(MazeTile{HALL, x, y, 1, 1, 0});

    bool ok = true;
    while (ok) {
        ok = mazeWalk(&x, &y);
        if (!ok)
            ok = mazeHunt(&x, &y);
    }
}

std::array<char, 4> Maze::shuffleDirections() {
    std::array<char, 4> directions{
            'N',
            'S',
            'E',
            'W',
    };

    shuffle(directions.begin(), directions.end(), *m_rand_engine);
    return directions;
}

// Given a region_id, this will find the ultimate thing that it was
// connected to. For example, 5 might be connected to 9, which was connected to
// root.
//
// returns the original region_id if it couldn't find a region.
int Maze::findConnectedRegion(int region_id) {
    while (m_connections.count(region_id)) {
        region_id = m_connections[region_id];
    }
    return region_id;
}

void Maze::connectRegions() {
    // select a root region.
    m_root_region = -1;
    MazeTile current_tile{};
    current_tile.type = INVALID;

    while (m_root_region == -1) {
        auto x = getRandom(0, m_config.w / 2) * 2;
        auto y = getRandom(0, m_config.h / 2) * 2;

        auto t = getTile(x, y);
        if (t.type == ROOM) {
            m_root_region = t.region_id;
            SPDLOG_INFO(
                    "room at {},{} with region_id {} selected as m_root_region", x, y,
                    m_root_region);
            SPDLOG_INFO("{} total connectors available with {} total unconnected regions",
                        m_connectors.size(), m_unconnected_regions.size());
        }
    }

    auto root_connectors = m_region_connectors[m_root_region];
    shuffle(root_connectors.begin(), root_connectors.end(), *m_rand_engine);
    current_tile = root_connectors.front();

    shuffle(m_connectors.begin(), m_connectors.end(), *m_rand_engine);

    // try to connect every region with a single door
    while (m_unconnected_regions.size() > 0) {
        int first_region = 0;
        int second_region = 0;

        current_tile = getTile(current_tile.x,
                               current_tile.y); // update the copy of the data

        getConnectorRegions(current_tile.x, current_tile.y, &first_region,
                            &second_region);

        bool did_connect = false;

        // this connector is the same region on either side, but maybe we can
        // connect it.
        if (first_region == second_region) {
            if (getRandom(0, 1000) < 20 &&
                !isNextToDoor(current_tile.x, current_tile.y)) {
                current_tile.type = DOOR;
                current_tile.region_id = m_root_region;
                setTile(current_tile);
                did_connect = false; // we don't want to perform the merge clean
                // up here as this is already connected

                SPDLOG_INFO("connector at {},{} used randomly to connect "
                            "already regions",
                            current_tile.x, current_tile.y);
            }
        } else if (findConnectedRegion(first_region) == m_root_region) {
            // first_region was connected to the root region (or is root), we
            // preferentially connect them here and merge.

            current_tile.type = DOOR;
            current_tile.region_id =
                    m_root_region; // doors are always in the m_root_region
            setTile(current_tile);
            did_connect = true;
            first_region =
                    m_root_region; // optimization! if the region wasn't actually
            // root, we set this to root to avoid cycling
            // through previous connected regions.

            SPDLOG_INFO(
                    "connector at {},{} used to merge region {} to root region {}",
                    current_tile.x, current_tile.y, second_region, first_region);
        } else if (findConnectedRegion(second_region) == m_root_region) {
            // second_region was connected to root, so we switch the regions to
            // connect the first_region region to second_region (which is now
            // root)

            current_tile.type = DOOR;
            current_tile.region_id = m_root_region;
            setTile(current_tile);
            did_connect = true;

            // we always connect the second_region to the first; as our cleanup
            // code assumes that.
            second_region = first_region;
            first_region = m_root_region;

            SPDLOG_INFO(
                    "connector at {},{} used to merge region {} to root region {}",
                    current_tile.x, current_tile.y, second_region, first_region);
        } else if (getRandom(0, 1000) < 100 &&
                   !isNextToDoor(current_tile.x, current_tile.y)) {
            // neither of these regions are connected ultimately to root so we
            // merge them radomly just to give us more doors.
            current_tile.type = DOOR;
            current_tile.region_id = m_root_region;
            setTile(current_tile);
            did_connect = true; // we don't want to perform the merge clean up
            // here as this is already connected

            SPDLOG_INFO("connector at {},{} used to merge regions {} and {}",
                        current_tile.x, current_tile.y, first_region,
                        second_region);
        }

        // we connected some regions, so we need to do the conversion
        if (did_connect) {
            // update all the tiles with the old region to have the new region.
            updateRegion(second_region, first_region);

            // update the m_unconnected_regions set to remove this region as being
            // unconnected.
            m_unconnected_regions.erase(second_region);

            // mark the dead region as connected to the live region
            m_connections[second_region] = first_region;

            int tile_count = 0;
            int origin_tile_count = 0;

            // copy all the tiles for the dead region to the live regions's
            // tiles.
            for_each(m_region_tiles[second_region].begin(),
                     m_region_tiles[second_region].end(), [&](MazeTile t) {
                        if (t.x == 0 && t.y == 0) {
                            origin_tile_count++;
                        }
                        t.region_id = first_region;
                        m_region_tiles[first_region].push_back(t);
                        tile_count++;
                    });
            SPDLOG_INFO("we merged {} tiles and {} origin tiles from region "
                        "{} to region {}",
                        tile_count, origin_tile_count, second_region,
                        first_region);

            // delete the dead region from m_region_tiles
            m_region_tiles.erase(second_region);
        } else if (current_tile.type != DOOR) {
            // if this connector wasn't used to connect anything, lets remove it
            current_tile.type = WALL;
            current_tile.region_id = 0;
            setTile(current_tile);
        }

        // Get another connector

        if (root_connectors.size() != 0) {
            // lets see if we have any left in the rootm_connectors
            current_tile = root_connectors.front();
            root_connectors.erase(root_connectors.begin());
        } else {
            // otherwise pull one from the big pool
            if (m_connectors.size() == 0) {
                break; // Oh, we're done.
            }

            current_tile = m_connectors.front();
            m_connectors.erase(m_connectors.begin());
        }
    }
    SPDLOG_INFO("{} total connectors left with {} total unconnected regions",
                m_connectors.size(), m_unconnected_regions.size());
    m_connectors.clear();
}

void Maze::updateRegion(int old_region, int new_region) {
    int tile_count = 0;
    int origin_tile_count = 0;
    for_each(m_region_tiles[old_region].begin(), m_region_tiles[old_region].end(),
             [&](MazeTile t) {
                 if (t.type != WALL) // a wall isn't part of a region; we will
                     // want to skip this.
                 {
                     t.region_id = new_region;

                     if (t.x == 0 && t.y == 0) {
                         origin_tile_count++;
                     }
                     setTile(t);
                     tile_count++;
                 }
             });

    SPDLOG_INFO("updateRegion {} tiles and {} origin tiles to region_id {}",
                tile_count, origin_tile_count, new_region);
}

void Maze::placeRooms() {
    for (auto attempts = 0; attempts < m_config.room_attempts; attempts++) {
        auto room_width = getRandom(m_config.room_min, m_config.room_max) * 2 + 1;
        auto room_height = getRandom(m_config.room_min, m_config.room_max) * 2 + 1;
        auto room_x = getRandom(0, (m_config.w - room_width) / 2) * 2;
        auto room_y = getRandom(0, (m_config.h - room_height) / 2) * 2;

        if (!roomExists(room_x, room_y, room_width, room_height)) {
            SPDLOG_INFO("placing room region {} at {},{} - {},{}",
                        m_current_region, room_x, room_y, room_width,
                        room_height);
            for (auto x = room_x; x < room_x + room_width; x++) {
                for (auto y = room_y; y < room_y + room_height; y++) {
                    setTile(MazeTile{ROOM, x, y, room_width, room_height,
                                     m_current_region});
                }
            }
            m_current_region++;
        }
    }
}

bool Maze::roomExists(int x, int y, int width, int height) {
    for (auto scan_x = x - 1; scan_x < x + width + 1; scan_x += 1) {
        for (auto scan_y = y - 1; scan_y < y + height + 1; scan_y += 1) {
            auto tile = getTile(scan_x, scan_y);
            if (tile.type == ROOM) {
                return true;
            }
        }
    }
    return false;
}

// true = success
bool Maze::mazeWalk(int *x, int *y) {
    auto directions = shuffleDirections();

    for (auto d = 0; d < 4; d++) {
        auto face = directions[d];
        auto tile = getAtDirection(*x, *y, face, 2);
        if (tile.type == WALL) {
            carveToDirection(x, y, face, 2, MazeTile{HALL, 0, 0, 0, 0, 0});
            // we are now there
            return true;
        }
    }

    // could not find a path
    return false;
}

// hunt for a previously visited location, that has an unvisited neighbour
bool Maze::mazeHunt(int *x, int *y) {
    vector<int> incomplete_rows;

    // construct a vector of pointers to each row that will indicate
    // whether this row should be scanned again. We shuffle them and
    // use them in random order.
    for (int ay = 0; ay < m_config.h; ay += 2) {
        incomplete_rows.push_back(ay);
        shuffle(incomplete_rows.begin(), incomplete_rows.end(), *m_rand_engine);
    }

    while (incomplete_rows.size() > 0) {
        // grab the first row and process it
        auto scan_y = incomplete_rows.back();
        int scan_x;

        for (scan_x = 0; scan_x < m_config.w; scan_x += 2) {
            auto t = getTile(scan_x, scan_y);
            if (t.type == HALL) {
                if (getTile(scan_x - 2, scan_y).type == WALL) {
                    *x = scan_x;
                    *y = scan_y;
                    carveToDirection(x, y, 'W', 2, t);
                    return true; // go back to the walk
                }

                if (getTile(scan_x + 2, scan_y).type == WALL) {
                    *x = scan_x;
                    *y = scan_y;
                    carveToDirection(x, y, 'E', 2, t);
                    return true; // go back to the walk
                }

                if (getTile(scan_x, scan_y - 2).type == WALL) {
                    *x = scan_x;
                    *y = scan_y;
                    carveToDirection(x, y, 'N', 2, t);
                    return true; // go back to the walk
                }

                if (getTile(scan_x, scan_y + 2).type == WALL) {
                    *x = scan_x;
                    *y = scan_y;
                    carveToDirection(x, y, 'S', 2, t);
                    return true; // go back to the walk
                }
            }
        }
        // we found nothing usable on this row, erase it.
        if (scan_x >= m_config.w) {
            // found a hallway but nothing I could use
            incomplete_rows.pop_back();
        }
    }

    m_current_region++;
    return false;
}


int Maze::getRandom(int start, int end) {
    std::uniform_int_distribution<int> r(start, end);
    return r(*m_rand_engine);
}

void Maze::setTile(MazeTile t) {
    if (t.x < 0 || t.x > m_config.w - 1 || t.y < 0 || t.y > m_config.h - 1)
        return;

    m_tile_map[t.x + m_config.w * t.y] = t;
}

MazeTile Maze::getTile(int x, int y) {
    if (x < 0 || x > m_config.w - 1 || y < 0 || y > m_config.h - 1) {
        return MazeTile{INVALID, 0, 0, 0, 0, 0};
    }

    return m_tile_map[x + m_config.w * y];
}

MazeTile Maze::getAtDirection(int x, int y, char face, int distance) {
    switch (face) {
        case 'N':
            y = y - distance;
            break;
        case 'E':
            x = x + distance;
            break;
        case 'S':
            y = y + distance;
            break;
        case 'W':
            x = x - distance;
            break;
        default:
            return MazeTile{INVALID, 0, 0, 0, 0, 0};
    }

    return getTile(x, y);
}

void Maze::carveToDirection(int *x, int *y, char face, int distance, MazeTile t) {
    auto tx = *x;
    auto ty = *y;

    t.region_id = m_current_region;
    t.h = 1;
    t.w = 1;

    switch (face) {
        case 'N':
            for (ty = *y; ty > *y - distance; ty--) {
                t.x = *x;
                t.y = ty;
                setTile(t);
            }
            break;
        case 'E':
            for (tx = *x; tx < *x + distance; tx++) {
                t.x = tx;
                t.y = *y;
                setTile(t);
            }
            break;
        case 'S':
            for (ty = *y; ty < *y + distance; ty++) {
                t.x = *x;
                t.y = ty;
                setTile(t);
            }
            break;
        case 'W':
            for (tx = *x; tx > *x - distance; tx--) {
                t.x = tx;
                t.y = *y;
                setTile(t);
            }
            break;
    }

    *x = tx;
    *y = ty;

    t.x = *x;
    t.y = *y;

    setTile(t);
}
