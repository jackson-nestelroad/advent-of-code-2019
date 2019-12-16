#include "../AdventOfCode2019.hpp"

using coordinate = std::pair<int, int>;
using fraction = std::pair<int, int>;

int gcd(int a, int b) {
    a = std::abs(a);
    b = std::abs(b);
    while (b) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

fraction reduce(int num, int denom) {
    int div = gcd(num, denom);
    return { num / div, denom / div };
}

namespace std {
    template <>
    struct hash<std::pair<int, int>> {
        size_t operator()(const std::pair<int, int> &val) const noexcept {
            return hash<int>{ }(val.first) ^ hash<int>{ }(val.second);
        }
    };
}

enum AsteroidBelt {
    Asteroid = '#',
    Empty = '.'
};

int AoC::A::day10() {
    auto grid = in::readFile<in::line<>>("input/10.txt");
    std::vector<coordinate> asteroids;
    const std::size_t rows = grid.size();
    const std::size_t cols = grid[0].length();
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (grid[y][x] == Asteroid) {
                asteroids.emplace_back(x, y);
            }
        }
    }

    std::unordered_map<coordinate, int> numDetections;
    for (auto asteroid : asteroids) {
        std::unordered_set<fraction> detections;
        for (auto other : asteroids) {
            if (asteroid == other) {
                continue;
            }
            int dx = other.first - asteroid.first;
            int dy = other.second - asteroid.second;
            detections.insert(reduce(dy, dx));
        }
        numDetections.insert({ asteroid, static_cast<int>(detections.size()) });
    }

    auto max = std::max_element(numDetections.begin(), numDetections.end(), [](auto a, auto b) {
        return a.second < b.second;
    });

    return max->second;
}

int AoC::B::day10() {
    // Part B relies on the answer from Part A
    auto grid = in::readFile<in::line<>>("input/10.txt");
    std::vector<coordinate> asteroids;
    const std::size_t rows = grid.size();
    const std::size_t cols = grid[0].length();
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (grid[y][x] == Asteroid) {
                asteroids.emplace_back(x, y);
            }
        }
    }

    std::unordered_map<coordinate, int> numDetections;
    for (auto asteroid : asteroids) {
        std::unordered_set<fraction> detections;
        for (auto other : asteroids) {
            if (asteroid == other) {
                continue;
            }
            int dx = other.first - asteroid.first;
            int dy = other.second - asteroid.second;
            detections.insert(reduce(dy, dx));
        }
        numDetections.insert({ asteroid, static_cast<int>(detections.size()) });
    }

    auto max = std::max_element(numDetections.begin(), numDetections.end(), [](const auto &a, const auto &b) {
        return a.second < b.second;
    });

    // Part B begins here
    const auto station = max->first;
    
    // Remove station from consideration
    auto newEnd = std::remove_if(asteroids.begin(), asteroids.end(), [station](const auto &a) {
        return a == station;
    });
    asteroids.erase(newEnd, asteroids.end());

    // Transform all points to make the station the origin (0, 0)
    std::transform(asteroids.begin(), asteroids.end(), asteroids.begin(), [station](const auto &a) -> coordinate {
        return { a.first - station.first, station.second - a.second };
    });

    // Collection of lambdas to sort coordinates in clockwise order starting from pi/2 = 90 deg
    constexpr double PI = 3.1415926535;
    constexpr int precision = 10'000;
    constexpr int start = static_cast<int>((PI / 2) * precision);

    const auto rotate = [start](int a) constexpr -> int {
        return a <= start ? -a : 4 * start - a;
    };

    const auto clockwiseSort = [rotate](int a, int b) -> bool {
        return rotate(a) < rotate(b);
    };

    // Move all asteroids into a bucket based on their angle from center station
    std::map<int, std::list<coordinate>, decltype(clockwiseSort)> angleMap(clockwiseSort);
    for (auto asteroid : asteroids) {
        int angle = static_cast<int>(std::atan2(asteroid.second, asteroid.first) * precision);
        auto it = angleMap.find(angle);
        angleMap[angle].push_back(asteroid);
    }

    // Destroy asteroids in clockwise rotation
    constexpr int wanted = 200;
    int destroyed = 0;
    while (destroyed < wanted) {
        for (auto &angle : angleMap) {
            // Get asteroid closest to the station (closest to (0,0))
            auto min = std::min_element(angle.second.begin(), angle.second.end(), [](const auto &a, const auto &b) {
                int absA1 = std::abs(a.first);
                int absA2 = std::abs(a.second);
                int absB1 = std::abs(b.first);
                int absB2 = std::abs(b.second);
                return std::tie(absA1, absA2) < std::tie(absB1, absB2);
            });
            if (!angle.second.empty()) {
                if (destroyed == wanted - 1) {
                    return (min->first + station.first) * 100 + (station.second - min->second);
                }
                else {
                    angle.second.erase(min);
                    ++destroyed;
                }
            }
        }
    }
    // Hopefully we will never reach here
    return -1;
}