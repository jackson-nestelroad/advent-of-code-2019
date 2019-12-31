#include "../AdventOfCode2019.hpp"

enum class map_tile {
    wall = '#',
    open = '.'
};

namespace position {
    struct coordinate {
        std::size_t x;
        std::size_t y;
        std::size_t z;
    };

    bool operator<(const coordinate &a, const coordinate &b) {
        return std::tie(a.x, a.y, a.z) < std::tie(b.x, b.y, b.z);
    }

    bool operator==(const coordinate &a, const coordinate &b) {
        return std::tie(a.x, a.y, a.z) == std::tie(b.x, b.y, b.z);
    }

    enum class direction {
        left = 0,
        right = 1,
        up = 2,
        down = 3
    };
    
    constexpr std::size_t directions = 4;

    direction invertDirection(direction dir) {
        return static_cast<direction>(static_cast<int>(dir) ^ 1);
    }

    coordinate move(const coordinate &pos, direction dir) {
        switch (dir) {
            case direction::left: return { pos.x - 1, pos.y, pos.z };
            case direction::right: return { pos.x + 1, pos.y, pos.z };
            case direction::up: return { pos.x, pos.y - 1, pos.z };
            case direction::down: return { pos.x, pos.y + 1, pos.z };
        }
        return pos;
    }

    std::vector<coordinate> move(const coordinate &pos) {
        return {
            { pos.x - 1, pos.y, pos.z },
            { pos.x + 1, pos.y, pos.z },
            { pos.x, pos.y - 1, pos.z },
            { pos.x, pos.y + 1, pos.z }
        };
    }
}

struct recursive_portal {
    position::coordinate dest;
    bool inner;
};

using pluto_map = std::vector<std::string>;
using portal_map = std::unordered_map<position::coordinate, position::coordinate>;
using recursive_portal_map = std::unordered_map<position::coordinate, recursive_portal>;
using named_location_map = std::unordered_map<std::string, position::coordinate>;

// Adopted from boost/functional/hash/hash.hpp
template <typename T>
inline void hash_combine(std::size_t &seed, const T &value) {
    std::hash<T> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
    template <>
    struct hash<position::coordinate> {
        std::size_t operator()(const position::coordinate &data) const {
            std::size_t seed { };
            hash_combine(seed, data.x);
            hash_combine(seed, data.y);
            hash_combine(seed, data.z);
            return seed;
        }
    };
}


inline bool validPosition(const position::coordinate &pos, std::size_t rows, std::size_t cols) {
    return !(pos.y < 0 || pos.y >= rows || pos.x < 0 || pos.x >= cols);
}

void findPortals(const pluto_map &map, std::size_t rows, std::size_t cols, portal_map &portals, named_location_map &locations) {
    std::unordered_set<position::coordinate> observed;
    std::unordered_map<std::string, std::pair<position::coordinate, position::direction>> namedPortalMap;

    // Regex to find all letters in a string
    std::regex isLetter("[A-Z]");
    for (std::size_t y = 0; y < rows; ++y) {
        // Look for all letters in the current row
        auto match = std::sregex_iterator(map[y].begin(), map[y].end(), isLetter);
        auto end = std::sregex_iterator();
        for (match; match != end; ++match) {
            std::size_t x = match->position();
            position::coordinate pos { x, y };

            // We have not seen this location before
            if (observed.find(pos) == observed.end()) {
                observed.insert(pos);
                    
                position::coordinate other;
                bool posIsInner = false;
                position::direction dirMoved;
                std::string p;
                for (int dir = 0; dir < position::directions; ++dir) {
                    auto next = position::move(pos, static_cast<position::direction>(dir));
                    if (validPosition(next, rows, cols)) {
                        auto at = map[next.y][next.x];
                        if (std::regex_match(std::string(1, at), isLetter)) {
                            observed.insert(next);
                            if (pos < next) {
                                p += map[y][x];
                                p += at;
                            }
                            else {
                                p += at;
                                p += map[y][x];
                            }
                            other = next;
                            dirMoved = static_cast<position::direction>(dir);
                        }
                        else if (static_cast<map_tile>(at) == map_tile::open) {
                            posIsInner = true;
                        }
                    }
                }
                const position::coordinate &inner = posIsInner ? (dirMoved = position::invertDirection(dirMoved), pos) : other;
                if (auto it = namedPortalMap.find(p); it != namedPortalMap.end()) {
                    portals.insert({ it->second.first, position::move(inner, dirMoved) });
                    portals.insert({ inner, position::move(it->second.first, it->second.second) });
                    namedPortalMap.erase(it);
                }
                else {
                    namedPortalMap.insert({ p, { inner, dirMoved } });
                }
            }
        }
    }

    // Transfer over all labeled locations that did not connect to another
    for (const auto &[name, data] : namedPortalMap) {
        locations.insert({ name, position::move(data.first, data.second) });
    }
}

void findPortals(const pluto_map &map, std::size_t rows, std::size_t cols, recursive_portal_map &portals, named_location_map &locations) {
    std::unordered_set<position::coordinate> observed;
    std::unordered_map<std::string, std::pair<position::coordinate, position::direction>> namedPortalMap;

    const position::coordinate center = { cols / 2, rows / 2 };

    // Regex to find all letters in a string
    std::regex isLetter("[A-Z]");
    for (std::size_t y = 0; y < rows; ++y) {
        // Look for all letters in the current row
        auto match = std::sregex_iterator(map[y].begin(), map[y].end(), isLetter);
        auto end = std::sregex_iterator();
        for (match; match != end; ++match) {
            std::size_t x = match->position();
            position::coordinate pos { x, y };

            // We have not seen this location before
            if (observed.find(pos) == observed.end()) {
                observed.insert(pos);

                position::coordinate other;
                bool posIsInner = false;
                position::direction dirMoved;
                std::string p;
                for (int dir = 0; dir < position::directions; ++dir) {
                    auto next = position::move(pos, static_cast<position::direction>(dir));
                    if (validPosition(next, rows, cols)) {
                        auto at = map[next.y][next.x];
                        if (std::regex_match(std::string(1, at), isLetter)) {
                            observed.insert(next);
                            if (pos < next) {
                                p += map[y][x];
                                p += at;
                            }
                            else {
                                p += at;
                                p += map[y][x];
                            }
                            other = next;
                            dirMoved = static_cast<position::direction>(dir);
                        }
                        else if (static_cast<map_tile>(at) == map_tile::open) {
                            posIsInner = true;
                        }
                    }
                }
                const position::coordinate &inner = posIsInner ? (dirMoved = position::invertDirection(dirMoved), pos) : other;

                bool innerLevel;
                if (dirMoved == position::direction::left || dirMoved == position::direction::right) {
                    innerLevel = (inner.x < center.x) ^ (dirMoved == position::direction::right);
                }
                else {
                    innerLevel = (inner.y < center.y) ^ (dirMoved == position::direction::down);
                }

                if (auto it = namedPortalMap.find(p); it != namedPortalMap.end()) {
                    portals.insert({ it->second.first, { position::move(inner, dirMoved), !innerLevel } });
                    portals.insert({ inner, { position::move(it->second.first, it->second.second), innerLevel } });
                    namedPortalMap.erase(it);
                }
                else {
                    namedPortalMap.insert({ p, { inner, dirMoved } });
                }
            }
        }
    }

    // Transfer over all labeled locations that did not connect to another
    for (const auto &[name, data] : namedPortalMap) {
        locations.insert({ name, position::move(data.first, data.second) });
    }
}

int AoC::A::day20() {
    // pluto_map map = in::readFile<in::line<>>("test/20_2.txt");
    // pluto_map map = in::readFile<in::line<>>("test/20_1.txt");
    pluto_map map = in::readFile<in::line<>>("input/20.txt");

    const std::size_t rows = map.size();
    const std::size_t cols = map[0].size();

    constexpr std::string_view entrance = "AA";
    constexpr std::string_view exit = "ZZ";

    portal_map portals;
    named_location_map locations;
    findPortals(map, rows, cols, portals, locations);

    const auto &entrancePos = locations.find(entrance.data())->second;
    const auto &exitPos = locations.find(exit.data())->second;

    // BFS away from entrance to find shortest path to exit
    std::unordered_map<position::coordinate, std::size_t> distances;
    std::queue<position::coordinate> toVisit;
    toVisit.push(entrancePos);
    distances[entrancePos] = 0;
    while (!toVisit.empty()) {
        const auto &pos = toVisit.front();
        auto distance = distances[pos];

        // Found exit
        if (pos == exitPos) {
            return static_cast<int>(distance);
        }

        // Only branch outwards if tile is an open part of the maze
        auto tile = map[pos.y][pos.x];
        if (static_cast<map_tile>(tile) == map_tile::open) {
            ++distance;
            for (auto &next : position::move(pos)) {
                // Took a portal to a new coordinate
                if (auto it = portals.find(next); it != portals.end()) {
                    next = it->second;
                }
                // Only add position if it has not been visited yet
                if (validPosition(next, rows, cols) && distances.find(next) == distances.end()) {
                    distances[next] = distance;
                    toVisit.push(next);
                }
            }
        }

        toVisit.pop();
    }

    console::fatal("Could not find path to exit marked ", exit);
    return -1;
}

int AoC::B::day20() {
    // Consider the "level" as the third z-coordinate

    // pluto_map map = in::readFile<in::line<>>("test/20_B_1.txt");
    // pluto_map map = in::readFile<in::line<>>("test/20_1.txt");
    pluto_map map = in::readFile<in::line<>>("input/20.txt");

    const std::size_t rows = map.size();
    const std::size_t cols = map[0].size();

    constexpr std::string_view entrance = "AA";
    constexpr std::string_view exit = "ZZ";

    // Recursive portal map considers the change in level each portal provides
    recursive_portal_map portals;
    named_location_map locations;
    findPortals(map, rows, cols, portals, locations);

    const auto &entrancePos = locations.find(entrance.data())->second;
    const auto &exitPos = locations.find(exit.data())->second;

    // BFS away from entrance to find shortest path to exit
    std::unordered_map<position::coordinate, std::size_t> distances;
    std::queue<position::coordinate> toVisit;
    toVisit.push(entrancePos);
    distances[entrancePos] = 0;
    while (!toVisit.empty()) {
        const auto &pos = toVisit.front();
        auto distance = distances[pos];

        // Found exit
        if (pos == exitPos) {
            return static_cast<int>(distance);
        }

        // Only branch outwards if tile is an open part of the maze
        auto tile = map[pos.y][pos.x];
        if (static_cast<map_tile>(tile) == map_tile::open) {
            ++distance;
            for (auto &next : position::move(pos)) {
                // Took a portal to a new coordinate
                if (auto it = portals.find({ next.x, next.y }); it != portals.end()) {
                    if (it->second.inner || next.z > 0) {
                        auto z = next.z + (it->second.inner ? 1 : -1);
                        next = it->second.dest;
                        next.z = z;
                    }
                }
                // Only add position if it has not been visited yet
                if (validPosition(next, rows, cols) && distances.find(next) == distances.end()) {
                    distances[next] = distance;
                    toVisit.push(next);
                }
            }
        }

        toVisit.pop();
    }

    console::fatal("Could not find path to exit marked ", exit);
    return -1;
}