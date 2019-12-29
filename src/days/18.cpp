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
    keychain keys;
};

// Adopted from boost/functional/hash/hash.hpp
template <typename T>
inline void hash_combine(std::size_t &seed, const T &value) {
    std::hash<T> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

inline keychain toKeyBit(key k) {
    return 1LL << (k - 'a');
}

// Adopted from https://blogs.msdn.microsoft.com/jeuge/2005/06/08/bit-fiddling-3/
std::size_t numKeys(keychain k) {
    std::size_t uCount;
    uCount = k - ((k >> 1) & 033333333333) - ((k >> 2) & 011111111111);
    return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

struct state {
    coordinate position;
    keychain doors;
    keychain keys;
    std::size_t distance;
    path visited;
};

struct key_path_state {
    key current;
    keychain collected;
};

bool operator==(const key_path_state &a, const key_path_state &b) {
    return std::tie(a.current, a.collected) == std::tie(b.current, b.collected);
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

    template <>
    struct hash<key_path_state> {
        std::size_t operator()(const key_path_state &data) const {
            std::size_t seed;
            hash_combine(seed, data.current);
            hash_combine(seed, data.collected);
            return seed;
        }
    };
}


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

// BFS away from the entrance to record all key locations and distances from entrance
// Returns a keychain with all keys found
keychain findKeysBFS(
    const vault_map &map, 
    const coordinate &entrance,
    std::unordered_map<key, key_pair_data> &initialGrabs,
    std::unordered_map<key, std::shared_ptr<state>> &keyLocations,
    std::unordered_map<key, std::unordered_map<key, key_pair_data>> &keyPairs
    ) {
    keychain found { };
    std::unordered_map<coordinate, std::shared_ptr<state>> visited;
    std::queue<std::shared_ptr<state>> toVisit;
    toVisit.push(std::make_shared<state>(state { entrance, 0, 0, 0, { } }));
    while (!toVisit.empty()) {
        auto currentState = toVisit.front();
        toVisit.pop();

        const auto &position = currentState->position;
        auto doors = currentState->doors;
        auto keys = currentState->keys;
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
                found |= toKeyBit(tile);
                keys |= toKeyBit(tile);
                keyLocations.insert({ tile, currentState });
                initialGrabs.insert({ tile, { distance, doors, keys } });
                // Map new key to all previous keys found
                for (const auto &[other, keyState] : keyLocations) {
                    auto &currentKeyMap = keyPairs[tile];
                    if (tile != other && currentKeyMap.find(other) == currentKeyMap.end()) {
                        // Find the last common location where the paths to the two keys combine
                        std::shared_ptr<state> commonState;
                        for (int i = static_cast<int>(keyState->distance); i >= 0; --i) {
                            if (keyState->visited[i] == pastPath[i]) {
                                commonState = visited[pastPath[i]];
                                break;
                            }
                        }
                        // Handle edge case where paths cross entrance in a non-optimal direction
                        if (commonState->position == entrance) {
                            if (keyState->distance >= 2 && keyState->visited[2].col == pastPath[2].col) {
                                commonState = visited[{ (keyState->visited[2].row + pastPath[2].row) / 2, pastPath[2].col }];
                            }
                        }
                        auto commonDistance = commonState->distance;
                        auto commonDoors = commonState->doors;
                        auto commonKeys = commonState->keys;

                        // Calculate distance between keys
                        auto distanceBetweenKeys = distance - commonDistance;
                        distanceBetweenKeys += keyState->distance - commonDistance;

                        // Calculate the keys required to move between the keys
                        keychain doorsBetweenKeys = (doors ^ commonDoors) | (keyState->doors ^ commonDoors);
                        keychain keysBetweenKeys = (keys ^ commonKeys) | (keyState->keys ^ commonKeys) | toKeyBit(other);

                        keyPairs[other][tile] = currentKeyMap[other] = { distanceBetweenKeys, doorsBetweenKeys, keysBetweenKeys };
                    }
                }
            }
        }
        ++distance;
        // Add poisitions we have not been to yet
        for (const auto &next : move(position)) {
            if (visited.find(next) == visited.end()) {
                toVisit.push(std::make_shared<state>(state { next, doors, keys, distance, pastPath }));
            }
        }
    }
    return found;
}

// A* implementation to find the shortest path to the finishedState moving across each key state in keyPairs
int findShortestPathToAllKeys(
    const std::function<std::size_t(const key_path_state &)> &h,
    const keychain finishedState,
    const std::unordered_map<key, key_pair_data> &initialGrabs,
    const std::unordered_map<key, std::unordered_map<key, key_pair_data>> &keyPairs
    ) {
    std::unordered_map<key_path_state, key_path_state> previous;
    std::unordered_map<key_path_state, std::size_t> gScore;
    std::unordered_map<key_path_state, std::size_t> fScore;

    std::unordered_set<key_path_state> toVisit;

    // Insert all possible initial keys
    for (const auto &[firstKey, data] : initialGrabs) {
        if ((data.doors | data.keys) == data.keys) {
            key_path_state state = { firstKey, data.keys };
            auto g = data.distance;
            gScore[state] = g;
            fScore[state] = g + h(state);
            toVisit.insert(state);
        }
    }

    // A* implementation
    std::size_t statesVisited = 0;
    while (!toVisit.empty()) {
        // Get next state with minimum fScore
        auto state = std::min_element(toVisit.begin(), toVisit.end(), 
            [&fScore](const auto &a, const auto &b) {
                return fScore[a] < fScore[b];
            });
        ++statesVisited;

        // Found all keys, this is the minimum path
        if (state->collected == finishedState) {
            return static_cast<int>(gScore[*state]);
        }
        else {
            // Find all possible next states
            for (const auto &[nextKey, nextKeyData] : keyPairs.find(state->current)->second) {
                if (((state->collected & toKeyBit(nextKey)) == 0) && (((state->collected ^ nextKeyData.doors) & nextKeyData.doors) == 0)) {
                    // Calculate new gScore
                    auto tentative = gScore[*state] + nextKeyData.distance;
                    key_path_state next = { nextKey, state->collected | nextKeyData.keys };
                    if (auto it = gScore.find(next); it == gScore.end() ? true : (tentative < it->second)) {
                        previous[next] = *state;
                        gScore[next] = tentative;
                        fScore[next] = tentative + h(next);
                        if (toVisit.find(next) == toVisit.end()) {
                            toVisit.insert(next);
                        }
                    }
                }
            }
        }
        toVisit.erase(state);
    }
    return -1;
}

int AoC::A::day18() {
    // vault_map map = in::readFile<in::line<>>("test/18_3.txt");
    // vault_map map = in::readFile<in::line<>>("test/18_2.txt");
    vault_map map = in::readFile<in::line<>>("test/18_1.txt");
    // vault_map map = in::readFile<in::line<>>("input/18.txt");
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
    std::unordered_map<key, std::unordered_map<key, key_pair_data>> keyPairs;

    const keychain allCollected = findKeysBFS(map, entrance, initialGrabs, keyLocations, keyPairs);
    const auto totalNumKeys = keyLocations.size();

    // Heuristic function
    // Result will be sum of minimum distance from all uncollected key to any other uncollected key
    const auto h = [&keyPairs, totalNumKeys, allCollected](const key_path_state &state) -> std::size_t {
        // Get keychain of all uncollected keys
        const keychain uncollected = ~state.collected & allCollected;
        const auto &startMap = keyPairs[state.current];
        std::size_t res = 0;
        key curr = 'a';
                
        // For each uncollected key
        for (auto temp = uncollected; temp != 0; temp >>= 1, ++curr) {
            if (uncollected & 1) {
                // Get minimum distance to any other uncollected key
                if (auto it = startMap.find(curr); it != startMap.end()) {
                    std::size_t min = it->second.distance;
                    for (const auto &[to, toMap] : keyPairs) {
                        if (uncollected & toKeyBit(to)) {
                            if (auto it = toMap.find(curr); it != toMap.end()) {
                                if (it->second.distance < min) {
                                    min = it->second.distance;
                                }
                            }
                        }
                    }
                    res += min;
                }
            }
        }
        return res;
    };

    auto result = findShortestPathToAllKeys(h, allCollected, initialGrabs, keyPairs);
    if (result < 0) {
        console::fatal("Could not find path to all keys.");
    }
    return result;
}

int AoC::B::day18() {
    // This solution does not run this example case correctly, for it does not
    // consider keys/doors that are mutually dependent on each other across
    // quadrants. This edge case does not occur in the actual input.

    // vault_map map = in::readFile<in::line<>>("test/18_B_1.txt");
    vault_map map = in::readFile<in::line<>>("input/18.txt");
    const std::size_t rows = map.size();
    const std::size_t cols = map[0].length();

    // Find initial entrance
    auto entrance = findEntrance(map, rows);
    if (entrance.row == std::string::npos && entrance.col == std::string::npos) {
        console::fatal("Vault map has no entrance point marked by ", static_cast<char>(Tile::Entrance));
    }
    
    // Create and save the four entrances
    std::vector<coordinate> entrances;
    for (std::size_t row = entrance.row - 1; row <= entrance.row + 1; ++row) {
        for (std::size_t col = entrance.col - 1; col <= entrance.col + 1; ++col) {
            if (row == entrance.row || col == entrance.col) {
                map[row][col] = static_cast<char>(Tile::Wall);
            }
            else {
                map[row][col] = static_cast<char>(Tile::Entrance);
                entrances.push_back({ row, col });
            }
        }
    }

    std::size_t pathLength = 0;
    
    // Consider each quadrant independently
    for (const auto &entrance : entrances) {
        std::unordered_map<key, key_pair_data> initialGrabs;
        std::unordered_map<key, std::shared_ptr<state>> keyLocations;
        std::unordered_map<key, std::unordered_map<key, key_pair_data>> keyPairs;

        const keychain keysFound = findKeysBFS(map, entrance, initialGrabs, keyLocations, keyPairs);
        const keychain allCollected = static_cast<keychain>(-1);

        // Give all keys that can't be found in this quadrant
        // This will effectively ignore all doors that cannot be opened by this robot alone
        // This can be thought of as the robot waiting for another robot to collect the required key
        const keychain initial = allCollected ^ keysFound;
        for (auto &[firstKey, firstKeyData] : initialGrabs) {
            firstKeyData.keys = initial | firstKeyData.keys;
        }

        // Heuristic function
        const auto h = [&keyPairs = std::as_const(keyPairs), initial](const key_path_state &state) -> std::size_t {
            const keychain collected = state.collected ^ initial;
            const auto &startMap = keyPairs.find(state.current)->second;
            std::size_t res = 0;
            for (const auto &[from, fromMap] : keyPairs) {
                if ((collected & toKeyBit(from)) == 0) {
                    if (auto it = startMap.find(from); it != startMap.end()) {
                        std::size_t min = it->second.distance;
                        for (const auto &[to, toData] : keyPairs.find(from)->second) {
                            if (auto it = fromMap.find(to); it != fromMap.end()) {
                                if (it->second.distance < min) {
                                    min = it->second.distance;
                                }
                            }
                        }
                        res += min;
                    }
                }
            }
            return res;
        };

        pathLength += findShortestPathToAllKeys(h, allCollected, initialGrabs, keyPairs);
    }

    return static_cast<int>(pathLength);
}