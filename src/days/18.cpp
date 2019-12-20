#include "../AdventOfCode2019.hpp"

using vault_map = std::vector<std::string>;
using key = char;
using keychain = std::unordered_set<key>;

enum class Tile {
    Wall = '#',
    Open = '.',
    Entrance = '@'
};

struct key_pair {
    key from;
    key to;
};

bool operator==(const key_pair &a, const key_pair &b) {
    return std::tie(a.from, a.to) == std::tie(b.from, b.to);
}

struct key_pair_data {
    std::size_t distance;
    keychain doors;
};

struct coordinate {
    std::size_t row;
    std::size_t col;
};

static bool operator==(const coordinate &a, const coordinate &b) {
    return std::tie(a.row, a.col) == std::tie(b.row, b.col);
}

struct state {
    coordinate position;
    keychain keys;
    keychain doors;
    std::size_t distance;
};

// Adopted from boost/functional/hash/hash.hpp
template <typename T>
inline void hash_combine(std::size_t &seed, const T &value) {
    std::hash<T> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
    template <>
    struct hash<coordinate> {
        std::size_t operator()(const coordinate &data) const {
            std::size_t seed = 0;
            hash_combine(seed, data.row);
            hash_combine(seed, data.col);
            return seed;
        }
    };

    template <>
    struct hash<key_pair> {
        std::size_t operator()(const key_pair &data) const {
            std::size_t seed = 0;
            hash_combine(seed, data.from);
            hash_combine(seed, data.to);
            return seed;
        }
    };
}

coordinate findEntrance(const vault_map &map, std::size_t rows) {
    for (std::size_t i = 0; i < rows; ++i) {
        auto entrance = map[i].find(static_cast<char>(Tile::Entrance));
        if (entrance != std::string::npos) {
            return { i, entrance };
        }
    }
    return { std::string::npos, std::string::npos };
}

std::vector<coordinate> move(const coordinate &pos) {
    return {
        { pos.row - 1, pos.col },
        { pos.row + 1, pos.col },
        { pos.row, pos.col - 1 },
        { pos.row, pos.col + 1 }
    };
}

int AoC::A::day18() {
    vault_map map = in::readFile<in::line<>>("test/18_1.txt");
    const std::size_t rows = map.size();
    const std::size_t cols = map[0].length();

    auto entrance = findEntrance(map, rows);
    if (entrance.row == std::string::npos && entrance.col == std::string::npos) {
        console::fatal("Vault map has no entrance point marked by ", static_cast<char>(Tile::Entrance));
    }

    std::unordered_map<key, coordinate> keyLocations;
    std::unordered_map<key_pair, key_pair_data> keyPairs;

    // BFS away from the entrance to record all key locations and distances from entrance
    std::unordered_set<coordinate> visited;
    std::queue<state> toVisit;
    toVisit.push({ entrance, { }, { }, 0 });
    while (!toVisit.empty()) {
        auto [position, keys, doors, distance] = toVisit.front();
        toVisit.pop();
        
        // Already been here before
        if (visited.find(position) != visited.end()) {
            continue;
        }
        visited.insert(position);

        auto tile = map[position.row][position.col];
        switch (static_cast<Tile>(tile)) {
            case Tile::Wall: continue;
            case Tile::Open: break;
            case Tile::Entrance: break;
            // Found a door or a key
            default:
                // Door
                if (std::isupper(tile)) {
                    doors.insert(std::tolower(tile));
                }
                // Key
                else {
                    keyLocations[tile] = position;
                    keyPairs[{ static_cast<char>(Tile::Entrance), tile }] = { distance, doors };
                    // All past keys can reach this key from the same path
                    for (auto key : keys) {
                        // Remove distance traveled before past key
                        auto &[prevDist, prevDoors] = keyPairs[{ static_cast<char>(Tile::Entrance), key }];
                        auto subDist = distance - prevDist;
                        // Remove doors encountered before the past key
                        keychain subDoors;
                        std::copy_if(doors.begin(), doors.end(), std::inserter(subDoors, subDoors.begin()), [&](const auto &door) {
                            return prevDoors.find(door) == prevDoors.end();
                        });
                        keyPairs[{ tile, key }] = keyPairs[{ key, tile }] = { subDist, subDoors };
                    }
                    keys.insert(tile);
                }
        }

        ++distance;
        for (const auto &next : move(position)) {
            toVisit.push({ next, keys, doors, distance });
        }
    }

    // Calculate all other key pairs
    for (const auto &[key1, start] : keyLocations) {
        for (const auto &[key2, dest] : keyLocations) {
            if (key1 != key2 && keyPairs.find({ key1, key2 }) == keyPairs.end()) {
                // Dijkstra from start to dest
            }
        }
    }

    return static_cast<int>(keyPairs.size());
}

int AoC::B::day18() {
    return 0;
}