#include "Maze.h"

/*
 * TODO: rewrite
 */

using namespace std;

sf::Color getColor(char c) {
    sf::Color colors[] = {
            sf::Color(50, 50, 50),
            sf::Color(190, 190, 20),
            sf::Color(220, 220, 220),
    };

    return colors[c];
}

Maze::Maze(MazeConfig *mazeConfig) {
    tile_map = nullptr;

    // Seed the random number generator
    std::random_device rd;
    rand_engine = make_unique<std::default_random_engine>(rd());

    setConfig(mazeConfig);
}

Maze::~Maze() { free(tile_map); }

void Maze::setConfig(MazeConfig *mazeConfig) {
    renderMutex.lock();
    memcpy(&config, mazeConfig, sizeof(MazeConfig));

    if (tile_map != nullptr)
        free(tile_map);

    this->tile_map = (MazeTile *) calloc(config.w * config.h, sizeof(MazeTile));

    rand_engine = make_unique<std::default_random_engine>(config.seed);
    dirty = true;

    total_regions = 0;
    current_region = 0;
    unconnected_regions.clear();
    region_colors.clear();
    region_tiles.clear();
    region_connectors.clear();
    connections.clear();
    connectors.clear();

    renderMutex.unlock();
}

void Maze::render(sf::RenderTarget *target) {
    renderMutex.lock();

    dirty = false;
    tile_shapes.clear();

    for (int y = 0; y < config.h; y++) {
        for (int x = 0; x < config.w; x++) {
            auto t = getTile(x, y);
            if (t.type == CONNECTOR) {
                auto shape =
                        make_shared<sf::RectangleShape>(sf::Vector2f(25.f, 25.f));

                shape->setFillColor(region_colors[t.region_id]);
                shape->setPosition(x * 25.f, y * 25.f);

                tile_shapes.push_back(shape);

                shape = make_shared<sf::RectangleShape>(sf::Vector2f(5.f, 5.f));
                shape->setFillColor(sf::Color::Yellow);
                shape->setPosition(10.f + (x * 25), 10.f + (y * 25));

                tile_shapes.push_back(shape);
            } else if (t.type == DOOR) {
                auto shape =
                        make_shared<sf::RectangleShape>(sf::Vector2f(25.f, 25.f));

                shape->setFillColor(region_colors[t.region_id]);
                shape->setPosition(x * 25.f, y * 25.f);

                tile_shapes.push_back(shape);

                shape = make_shared<sf::RectangleShape>(sf::Vector2f(5.f, 5.f));
                shape->setFillColor(sf::Color::Black);
                shape->setPosition(10.f + (x * 25), 10.f + (y * 25));

                tile_shapes.push_back(shape);
            } else {
                auto shape =
                        make_shared<sf::RectangleShape>(sf::Vector2f(25.f, 25.f));

                shape->setFillColor(region_colors[t.region_id]);
                shape->setPosition(x * 25.f, y * 25.f);

                tile_shapes.push_back(shape);
            }
        }
    }
    target->clear();
    for_each(tile_shapes.begin(), tile_shapes.end(),
             [&](shared_ptr<sf::RectangleShape> s) { target->draw(*s); });

    renderMutex.unlock();
}

bool Maze::shouldRender() { return dirty; }

void Maze::generate() {
    region_colors[0] = sf::Color(20, 20, 20);

    SPDLOG_INFO("generating with seed {}", config.seed);
    current_region = 1;
    region_colors[1] = getRandomColor();

    SPDLOG_INFO("placing rooms");
    placeRooms();

    SPDLOG_INFO("placing hallways");
    placeHallways();

    auto found_walls = scanForWalls();
    while (found_walls) {
        found_walls = scanForWalls();
    }

    SPDLOG_INFO("placing connectors");
    placeConnectors();

    SPDLOG_INFO("connecting regions");
    connectRegions();

    SPDLOG_INFO("removing dead ends");
    removeDeadEnds();

    dirty = true;
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

    for (int y = 0; y < config.h; y += 1) {
        for (int x = 0; x < config.w; x += 1) {
            auto tile = getTile(x, y);
            if (tile.type == HALL || tile.type == DOOR) {
                if (isDeadEnd(x, y)) {
                    dead_ends.push_back(tile);
                }
            }
        }
    }
    shuffle(dead_ends.begin(), dead_ends.end(), *rand_engine);

    return dead_ends;
}

// scan for any tiles that are dead ends
void Maze::removeDeadEnds() {

    int removed = 0;

    while (removed < config.culling) {
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
    for (auto x = 0; x < config.w; x += 2) {
        for (auto y = 0; y < config.h; y += 2) {
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
        x = getRandom(0, config.w / 2) * 2;
        y = getRandom(0, config.h / 2) * 2;
        if (getTile(x, y).type != ROOM)
            room_exists = false;
    }
    startWalking(x, y);
}

// build a list of all the possible connectors.
void Maze::placeConnectors() {
    for (int x = 0; x < config.w; x++) {
        for (int y = 0; y < config.h; y++) {
            auto tile = getTile(x, y);

            // keep a map of all the regions and their tiles for later
            region_tiles[tile.region_id].push_back(tile);

            // only walls can become connectors
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

                region_connectors[region1].push_back(connector_tile);
                region_connectors[region2].push_back(connector_tile);
                unconnected_regions.insert(region1);
                unconnected_regions.insert(region2);
                connectors.push_back(connector_tile);
            }
        }
    }

    SPDLOG_INFO("{} regions found with connectors",
                unconnected_regions.size());
    total_regions = unconnected_regions.size();
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

    shuffle(directions.begin(), directions.end(), *rand_engine);
    return directions;
}

// Given a region_id, this will find the ultimate thing that it was
// connected to. For example, 5 might be connected to 9, which was connected to
// root.
//
// returns the original region_id if it couldn't find a region.
int Maze::findConnectedRegion(int region_id) {
    while (connections.count(region_id)) {
        region_id = connections[region_id];
    }
    return region_id;
}

void Maze::connectRegions() {
    // select a root region.
    root_region = -1;
    MazeTile current_tile = MazeTile{INVALID};
    while (root_region == -1) {
        auto x = getRandom(0, config.w / 2) * 2;
        auto y = getRandom(0, config.h / 2) * 2;

        auto t = getTile(x, y);
        if (t.type == ROOM) {
            root_region = t.region_id;
            SPDLOG_INFO(
                    "room at {},{} with region_id {} selected as root_region", x, y,
                    root_region);
            SPDLOG_INFO("{} total connectors available with {} total "
                        "unconnected regions",
                        connectors.size(), unconnected_regions.size());
            region_colors[t.region_id] = sf::Color(220, 220, 220);

            dirty = true;
        }
    }

    auto root_connectors = region_connectors[root_region];
    shuffle(root_connectors.begin(), root_connectors.end(), *rand_engine);
    current_tile = root_connectors.front();

    shuffle(connectors.begin(), connectors.end(), *rand_engine);

    // try to connect every region with a single door
    while (unconnected_regions.size() > 0) {
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
                current_tile.region_id = root_region;
                setTile(current_tile);
                did_connect = false; // we don't want to perform the merge clean
                // up here as this is already connected

                SPDLOG_INFO("connector at {},{} used randomly to connect "
                            "already regions",
                            current_tile.x, current_tile.y);
            }
        } else if (findConnectedRegion(first_region) == root_region) {
            // first_region was connected to the root region (or is root), we
            // preferentially connect them here and merge.

            current_tile.type = DOOR;
            current_tile.region_id =
                    root_region; // doors are always in the root_region
            setTile(current_tile);
            did_connect = true;
            first_region =
                    root_region; // optimization! if the region wasn't actually
            // root, we set this to root to avoid cycling
            // through previous connected regions.

            SPDLOG_INFO(
                    "connector at {},{} used to merge region {} to root region {}",
                    current_tile.x, current_tile.y, second_region, first_region);
        } else if (findConnectedRegion(second_region) == root_region) {
            // second_region was connected to root, so we switch the regions to
            // connect the first_region region to second_region (which is now
            // root)

            current_tile.type = DOOR;
            current_tile.region_id = root_region;
            setTile(current_tile);
            did_connect = true;

            // we always connect the second_region to the first; as our cleanup
            // code assumes that.
            second_region = first_region;
            first_region = root_region;

            SPDLOG_INFO(
                    "connector at {},{} used to merge region {} to root region {}",
                    current_tile.x, current_tile.y, second_region, first_region);
        } else if (getRandom(0, 1000) < 100 &&
                   !isNextToDoor(current_tile.x, current_tile.y)) {
            // neither of these regions are connected ultimately to root so we
            // merge them radomly just to give us more doors.
            current_tile.type = DOOR;
            current_tile.region_id = root_region;
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

            // update the unconnected_regions set to remove this region as being
            // unconnected.
            unconnected_regions.erase(second_region);

            // mark the dead region as connected to the live region
            connections[second_region] = first_region;

            int tile_count = 0;
            int origin_tile_count = 0;

            // copy all the tiles for the dead region to the live regions's
            // tiles.
            for_each(region_tiles[second_region].begin(),
                     region_tiles[second_region].end(), [&](MazeTile t) {
                        if (t.x == 0 && t.y == 0) {
                            origin_tile_count++;
                        }
                        t.region_id = first_region;
                        region_tiles[first_region].push_back(t);
                        tile_count++;
                    });
            SPDLOG_INFO("we merged {} tiles and {} origin tiles from region "
                        "{} to region {}",
                        tile_count, origin_tile_count, second_region,
                        first_region);

            // delete the dead region from region_tiles
            region_tiles.erase(second_region);
        } else if (current_tile.type != DOOR) {
            // if this connector wasn't used to connect anything, lets remove it
            current_tile.type = WALL;
            current_tile.region_id = 0;
            setTile(current_tile);
        }

        // Get another connector

        if (root_connectors.size() != 0) {
            // lets see if we have any left in the root connectors
            current_tile = root_connectors.front();
            root_connectors.erase(root_connectors.begin());
        } else {
            // otherwise pull one from the big pool
            if (connectors.size() == 0) {
                break; // Oh, we're done.
            }

            current_tile = connectors.front();
            connectors.erase(connectors.begin());
        }
    }
    SPDLOG_INFO("{} total connectors left with {} total unconnected regions",
                connectors.size(), unconnected_regions.size());
    connectors.clear();
}

bool Maze::isUnconnected(int region_id) {
    return unconnected_regions.find(region_id) != unconnected_regions.end();
}

void Maze::updateRegion(int old_region, int new_region) {
    int tile_count = 0;
    int origin_tile_count = 0;
    for_each(region_tiles[old_region].begin(), region_tiles[old_region].end(),
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
    auto prev_sleep = config.sleep;
    config.sleep = 0;
    for (auto attempts = 0; attempts < config.room_attempts; attempts++) {
        auto room_width = getRandom(config.room_min, config.room_max) * 2 + 1;
        auto room_height = getRandom(config.room_min, config.room_max) * 2 + 1;
        auto room_x = getRandom(0, (config.w - room_width) / 2) * 2;
        auto room_y = getRandom(0, (config.h - room_height) / 2) * 2;

        if (!roomExists(room_x, room_y, room_width, room_height)) {
            SPDLOG_INFO("placing room region {} at {},{} - {},{}",
                        current_region, room_x, room_y, room_width,
                        room_height);
            for (auto x = room_x; x < room_x + room_width; x++) {
                for (auto y = room_y; y < room_y + room_height; y++) {
                    setTile(MazeTile{ROOM, x, y, room_width, room_height,
                                     current_region});
                }
            }
            current_region++;
            region_colors[current_region] = getRandomColor();
        }
    }
    config.sleep = prev_sleep;
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
            carveToDirection(x, y, face, 2, MazeTile{HALL});
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
    for (int ay = 0; ay < config.h; ay += 2) {
        incomplete_rows.push_back(ay);
        shuffle(incomplete_rows.begin(), incomplete_rows.end(), *rand_engine);
    }

    while (incomplete_rows.size() > 0) {
        // grab the first row and process it
        auto scan_y = incomplete_rows.back();
        int scan_x;

        for (scan_x = 0; scan_x < config.w; scan_x += 2) {
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
        if (scan_x >= config.w) {
            // found a hallway but nothing I could use
            incomplete_rows.pop_back();
        }
    }

    current_region++;
    region_colors[current_region] = getRandomColor();

    return false;
}

sf::Color Maze::getRandomColor() {
    std::uniform_int_distribution<unsigned short> rnd(20, 255);

    auto r = rnd(*rand_engine);
    auto g = rnd(*rand_engine);
    auto b = rnd(*rand_engine);

    return sf::Color((sf::Uint8) r, (sf::Uint8) g, (sf::Uint8) b);
}

int Maze::getRandom(int start, int end) {
    std::uniform_int_distribution<int> r(start, end);
    return r(*rand_engine);
}

void Maze::setTile(MazeTile t) {
    if (t.x < 0 || t.x > config.w - 1 || t.y < 0 || t.y > config.h - 1)
        return;

    dirty = true;
    tile_map[t.x + config.w * t.y] = t;

    if (config.sleep > 0) {
        sf::sleep(sf::milliseconds(config.sleep));
    }
}

MazeTile Maze::getTile(int x, int y) {
    if (x < 0 || x > config.w - 1 || y < 0 || y > config.h - 1) {
        return MazeTile{INVALID};
    }

    return tile_map[x + config.w * y];
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
            return MazeTile{INVALID};
    }

    return getTile(x, y);
}

void Maze::carveToDirection(int *x, int *y, char face, int distance, MazeTile t) {
    auto tx = *x;
    auto ty = *y;

    t.region_id = current_region;
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
