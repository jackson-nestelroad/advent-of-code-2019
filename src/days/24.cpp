#include "../AdventOfCode2019.hpp"

// Need 25 bits to hold compressed scan data
using scan = std::uint32_t;
using coord = std::pair<std::size_t, std::size_t>;
using coord3d = std::tuple<std::size_t, std::size_t, long long>;

template <std::size_t N>
inline scan toScanBit(std::size_t row, std::size_t col) {
    return 1ULL << (row * N + col);
}

template <std::size_t N>
inline bool isInfested(scan s, std::size_t row, std::size_t col) {
    return s & toScanBit<N>(row, col);
}

template <std::size_t N>
inline void infest(scan &s, std::size_t row, std::size_t col) {
    s |= toScanBit<N>(row, col);
}

template <std::size_t N>
inline void kill(scan &s, std::size_t row, std::size_t col) {
    s &= ~toScanBit<N>(row, col);
}

// Biodiversity rating is a unique number, so compress string data down to the bitwise rating
template <std::size_t N>
scan compress(const std::vector<std::string> &data) {
    scan res = 0;
    for (std::size_t i = 0; i < data.size(); ++i) {
        for (std::size_t j = 0; j < data[i].size(); ++j) {
            res |= data[i][j] == '#' ? toScanBit<N>(i, j) : 0;
        }
    }
    return res;
}

// Adopted from boost/functional/hash/hash.hpp
template <typename T>
static inline void hash_combine(std::size_t &seed, const T &value) {
    std::hash<T> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
    template <>
    struct hash<coord> {
        std::size_t operator()(const coord &data) const {
            std::size_t seed { };
            hash_combine(seed, data.first);
            hash_combine(seed, data.second);
            return seed;
        }
    };

    template <>
    struct hash<coord3d> {
        std::size_t operator()(const coord3d &data) const {
            std::size_t seed { };
            hash_combine(seed, std::get<0>(data));
            hash_combine(seed, std::get<1>(data));
            hash_combine(seed, std::get<2>(data));
            return seed;
        }
    };
}

namespace part1 {
    using adjacency_map = std::unordered_map<coord, std::vector<coord>>;

    template <std::size_t N>
    adjacency_map createAdjacencyMap() {
        adjacency_map map;
        for (std::size_t i = 0; i < N; ++i) {
            for (std::size_t j = 0; j < N; ++j) {
                auto &curr = map[{ i, j }];
                if (i > 0) {
                    curr.emplace_back(i - 1, j);
                }
                if (i < N - 1) {
                    curr.emplace_back(i + 1, j);
                }
                if (j > 0) {
                    curr.emplace_back(i, j - 1);
                }
                if (j < N - 1) {
                    curr.emplace_back(i, j + 1);
                }
            }
        }
        return map;
    }

    template <std::size_t N>
    scan transform(scan before, const adjacency_map &adjacent) {
        scan after = before;
        for (std::size_t i = 0; i < N; ++i) {
            for (std::size_t j = 0; j < N; ++j) {
                const auto &[curr, adjacentPairs] = *adjacent.find({ i, j });
                int count = 0;
                for (const auto &pair : adjacentPairs) {
                    count += isInfested<N>(before, pair.first, pair.second) ? 1 : 0;
                }
                if (isInfested<N>(before, curr.first, curr.second)) {
                    if (count != 1) {
                        kill<N>(after, curr.first, curr.second);
                    }
                }
                else {
                    if (count == 1 || count == 2) {
                        infest<N>(after, curr.first, curr.second);
                    }
                }
            }
        }
        return after;
    }
}

namespace part2 {
    using adjacency_map = std::unordered_map<coord3d, std::vector<coord3d>>;

    template <std::size_t N>
    constexpr std::size_t middleOfGrid = N / 2;

    template <std::size_t N>
    adjacency_map createAdjacencyMap() {
        // Third coordinate represents change in depth
        adjacency_map map;
        for (std::size_t row = 0; row < N; ++row) {
            for (std::size_t col = 0; col < N; ++col) {
                auto &curr = map[{ row, col, 0 }];
                // Top row, recurse out on top
                if (row == 0) {
                    curr.emplace_back(middleOfGrid<N> - 1, middleOfGrid<N>, -1);
                    curr.emplace_back(row + 1, col, 0);
                }
                // Bottom row, recurse out on bottom
                else if (row == N - 1) {
                    curr.emplace_back(row - 1, col, 0);
                    curr.emplace_back(middleOfGrid<N> + 1, middleOfGrid<N>, -1);
                }
                // Recurse down on bottom
                else if (col == middleOfGrid<N> && row == middleOfGrid<N> - 1) {
                    curr.emplace_back(row - 1, col, 0);
                    for (std::size_t k = 0; k < N; ++k) {
                        curr.emplace_back(0, k, 1);
                    }
                }
                // Recurse down on top
                else if (col == middleOfGrid<N> && row == middleOfGrid<N> + 1) {
                    for (std::size_t k = 0; k < N; ++k) {
                        curr.emplace_back(N - 1, k, 1);
                    }
                    curr.emplace_back(row + 1, col, 0);
                }
                else {
                    curr.emplace_back(row - 1, col, 0);
                    curr.emplace_back(row + 1, col, 0);
                }

                // First column, recurse out to the left
                if (col == 0) {
                    curr.emplace_back(middleOfGrid<N>, middleOfGrid<N> - 1, -1);
                    curr.emplace_back(row, col + 1, 0);
                }
                // Last column, recurse out to the right
                else if (col == N - 1) {
                    curr.emplace_back(row, col - 1, 0);
                    curr.emplace_back(middleOfGrid<N>, middleOfGrid<N> + 1, -1);
                }
                // Recurse down to the right
                else if (row == middleOfGrid<N> && col == middleOfGrid<N> - 1) {
                    curr.emplace_back(row, col - 1, 0);
                    for (std::size_t k = 0; k < N; ++k) {
                        curr.emplace_back(k, 0, 1);
                    }
                }
                // Recruse down to the left
                else if (row == middleOfGrid<N> && col == middleOfGrid<N> + 1) {
                    for (std::size_t k = 0; k < N; ++k) {
                        curr.emplace_back(k, N - 1, 1);
                    }
                    curr.emplace_back(row, col + 1, 0);
                }
                else {
                    curr.emplace_back(row, col - 1, 0);
                    curr.emplace_back(row, col + 1, 0);
                }
            }
        }
        map[{ middleOfGrid<N>, middleOfGrid<N>, 0 }].clear();
        return map;
    }

    template <std::size_t N>
    std::size_t bugsAfterTransformations(scan initial, const adjacency_map &adjacent, std::size_t iterations) {
        // Map to hold scanned data according to its depth
        std::unordered_map<long long, scan> currentData {
            { 0, initial },
        };

        // Number of transformations
        for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
            // nextData represents the next generation
            // currentData will not be changed until all updates have occurred in nextData
            auto nextData = currentData;
            // Get bounds of the depth that has been explored
            long long minDepth = std::min_element(currentData.begin(), currentData.end())->first;
            long long maxDepth = std::max_element(currentData.begin(), currentData.end())->first;
            // Iterate over each depth, increasing the bounds each time
            for (auto depth = minDepth - 1; depth <= maxDepth + 1; ++depth) {
                // Update the current depth - currentData[depth] => nextData[depth]
                auto currentScan = currentData[depth];
                for (std::size_t row = 0; row < N; ++row) {
                    for (std::size_t col = 0; col < N; ++col) {
                        const auto &[currentPair, adjacentCoords] = *adjacent.find({ row, col, 0 });
                        int count = 0;
                        for (const auto &[nextRow, nextCol, depthChange] : adjacentCoords) {
                            // May need to traverse to a new depth
                            count += isInfested<N>(depthChange != 0 ? currentData[depth + depthChange] : currentScan, nextRow, nextCol) ? 1 : 0;
                        }
                        if (isInfested<N>(currentScan, row, col)) {
                            if (count != 1) {
                                kill<N>(nextData[depth], row, col);
                            }
                        }
                        else {
                            if (count == 1 || count == 2) {
                                infest<N>(nextData[depth], row, col);
                            }
                        }
                    }
                }
            }
            // Update currentData once all changes are finished for the next iteration
            currentData = nextData;
        }

        // Loop over all data, adding up the total number of set bits
        long long minDepth = std::min_element(currentData.begin(), currentData.end())->first;
        long long maxDepth = std::max_element(currentData.begin(), currentData.end())->first;
        std::size_t numBugs = 0;
        for (auto depth = minDepth; depth <= maxDepth; ++depth) {
            numBugs += Integer::bitCount(currentData[depth]);
        }
        return numBugs;
    }
}

int AoC::A::day24() {
    auto input = in::readFile<in::line<>>("input/24.txt");
    static constexpr std::size_t N = 5;

    using namespace part1;
    auto adjacentMap = createAdjacencyMap<N>();

    std::unordered_set<scan> seen;
    scan current = compress<N>(input);
    while (seen.find(current) == seen.end()) {
        seen.insert(current);
        current = transform<N>(current, adjacentMap);
    }

    return static_cast<int>(current);
}

int AoC::B::day24() {
    auto input = in::readFile<in::line<>>("input/24.txt");
    static constexpr std::size_t N = 5;

    using namespace part2;
    auto adjacentMap = createAdjacencyMap<N>();
    return static_cast<int>(bugsAfterTransformations<N>(compress<N>(input), adjacentMap, 200));
}