#include "../AdventOfCode2019.hpp"

struct coordinate {
    std::size_t row;
    std::size_t col;
};

static bool operator==(const coordinate &a, const coordinate &b) {
    return std::tie(a.row, a.col) == std::tie(b.row, b.col);
}

using vault_map = std::vector<std::string>;
using key = char;
using keychain = uint32_t;
using path = std::vector<coordinate>;

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
            if (data.from < data.to) {
                hash_combine(seed, data.from);
                hash_combine(seed, data.to);
            }
            else {
                hash_combine(seed, data.to);
                hash_combine(seed, data.from);
            }
            
            return seed;
        }
    };
}

struct state {
    coordinate position;
    keychain doors;
    std::size_t distance;
    path visited;
};

enum class Tile {
    Wall = '#',
    Open = '.',
    Entrance = '@'
};

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

inline keychain toKeyBit(key k) {
    return 1LL << (k - 'a');
}

inline std::size_t toKeyCode(key k) {
    return k - 'a';
}

int AoC::A::day18() {
    // vault_map map = in::readFile<in::line<>>("test/18_1.txt");
    vault_map map = in::readFile<in::line<>>("input/18.txt");
    const std::size_t rows = map.size();
    const std::size_t cols = map[0].length();

    auto entrance = findEntrance(map, rows);
    if (entrance.row == std::string::npos && entrance.col == std::string::npos) {
        console::fatal("Vault map has no entrance point marked by ", static_cast<char>(Tile::Entrance));
    }

    // Maps entrance to distance to each key
    std::unordered_map<key, key_pair_data> initialGrabs;
    // Maps each key to its position
    std::unordered_map<key, std::shared_ptr<state>> keyLocations;
    // Maps each key to its distance and key requirements to each other key
    std::unordered_map<key_pair, key_pair_data> keyPairs;

    // BFS away from the entrance to record all key locations and distances from entrance
    std::unordered_map<coordinate, std::shared_ptr<state>> visited;
    std::queue<std::shared_ptr<state>> toVisit;
    toVisit.push(std::make_shared<state>(state { entrance, 0, 0, { } }));

    while (!toVisit.empty()) {
        auto currentState = toVisit.front();
        toVisit.pop();

        const auto &position = currentState->position;
        auto doors = currentState->doors;
        auto distance = currentState->distance;
        auto &pastPath = currentState->visited;

        pastPath.push_back(position);
        visited.insert({ position, currentState });

        auto tile = map[position.row][position.col];
        switch (static_cast<Tile>(tile)) {
            case Tile::Wall: continue;
            case Tile::Open: break;
            case Tile::Entrance: break;
            // Found a door or a key
            default:
                // Door
                if (std::isupper(tile)) {
                    doors |= toKeyBit(std::tolower(tile));
                }
                // Key
                else {
                    keyLocations.insert({ tile, currentState });
                    initialGrabs.insert({ tile, { distance, doors } });
                    // Map new key to all previous keys found
                    for (const auto &[key, keyState] : keyLocations) {
                        if (tile != key && keyPairs.find({ tile, key }) == keyPairs.end()) {
                            // Find the last common location where the paths to the two keys combine
                            std::shared_ptr<state> commonState;
                            for (int i = static_cast<int>(keyState->distance); i >= 0; --i) {
                                if (keyState->visited[i] == pastPath[i]) {
                                    commonState = visited[pastPath[i]];
                                    break;
                                }
                            }
                            auto commonDistance = commonState->distance;
                            auto commonDoors = commonState->doors;

                            // Calculate distance between keys
                            auto distanceBetweenKeys = distance - commonDistance;
                            distanceBetweenKeys += keyState->distance - commonDistance;

                            // Calculate the keys required to move between the keys
                            keychain doorsBetweenKeys = (doors ^ commonDoors) | (keyState->doors ^ commonDoors);

                            keyPairs.insert({ { tile, key }, { distanceBetweenKeys, doorsBetweenKeys } });
                        }
                    }
                }
        }
        ++distance;
        // Add poisitions we have not been to yet
        for (const auto &next : move(position)) {
            if (visited.find(next) == visited.end()) {
                toVisit.push(std::make_shared<state>(state { next, doors, distance, pastPath }));
            }
        }
    }

    return static_cast<int>(keyPairs.size());
}

int AoC::B::day18() {
    return 0;
}