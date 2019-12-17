#include "../AdventOfCode2019.hpp"

enum class Direction {
    North = 1,
    South = 2,
    West = 3,
    East = 4
};

enum class Status {
    Wall = 0,
    Open = 1,
    OxygenSystem = 2
};

struct coordinate {
    int x;
    int y;
};

struct tile {
    Status type;
    int distance;
};

bool operator==(const coordinate &a, const coordinate &b) {
    return std::tie(a.x, a.y) == std::tie(b.x, b.y);
}

bool operator<(const coordinate &a, const coordinate &b) {
    return std::tie(a.x, a.y) < std::tie(b.x, b.y);
}

coordinate move(const coordinate &pos, Direction dir) {
    coordinate updated = pos;
    switch (dir) {
        case Direction::North: updated.y += 1; break;
        case Direction::South: updated.y -= 1; break;
        case Direction::West: updated.x -= 1; break;
        case Direction::East: updated.x += 1; break;
        default: console::fatal("Unexpected direction given.");
    }
    return updated;
}

Direction opposite(Direction dir) {
    switch (dir) {
        case Direction::North: return Direction::South;
        case Direction::South: return Direction::North;
        case Direction::West: return Direction::East;
        case Direction::East: return Direction::West;
        default: console::fatal("Unexpected direction given.");
    }
    return Direction::North;
}

int AoC::A::day15() {
    auto input = in::readFile<in::value<IntCode::Int, ','>>("input/15.txt");
    IntCode::Computer comp { input };
    coordinate robotPos { 0, 0 };
    std::map<coordinate, tile> map { { robotPos, { Status::Open, 0 } } };

    constexpr Direction directions[4] {
        Direction::North,
        Direction::West,
        Direction::East,
        Direction::South,
    };

    // Moves the robot according to the IntCode program
    const auto moveRobot = [&](Direction dir) -> Status {
        comp << static_cast<IntCode::Int>(dir);
        comp.run();
        return static_cast<Status>(comp.popLastOutput());
    };

    // DFS function to explore area 
    const std::function<void(const coordinate &, Direction, int)> explore = [&](const coordinate &pos, Direction dir, int distance) {
        // Already been here at a short distance before
        if (auto data = map.find(pos); data != map.end() && distance >= data->second.distance) {
            return;
        }
        // Move robot and record result
        auto status = moveRobot(dir);
        map[pos] = { status, ++distance };
        // Robot hit a wall and did not actually move
        if (status == Status::Wall) {
            return;
        }
        // Move every possible direction
        for (auto newDir : directions) {
            explore(move(pos, newDir), newDir, distance);
        }
        // Backtrack the robot
        moveRobot(opposite(dir));
    };

    // Move out from starting position
    for (auto dir : directions) {
        explore(move(robotPos, dir), dir, 0);
    }

    auto oxygenSystem = std::find_if(map.begin(), map.end(), [](const auto &a) {
        return a.second.type == Status::OxygenSystem;
    });
    return oxygenSystem->second.distance;
}

int AoC::B::day15() {
    // Part B relies on Part A, so re-run DFS
    auto input = in::readFile<in::value<IntCode::Int, ','>>("input/15.txt");
    IntCode::Computer comp { input };
    coordinate robotPos { 0, 0 };
    std::map<coordinate, tile> map { { robotPos, { Status::Open, 0 } } };

    constexpr Direction directions[4] {
        Direction::North,
        Direction::West,
        Direction::East,
        Direction::South,
    };

    const auto moveRobot = [&](Direction dir) -> Status {
        comp << static_cast<IntCode::Int>(dir);
        comp.run();
        return static_cast<Status>(comp.popLastOutput());
    };

    const std::function<void(const coordinate &, Direction, int)> explore = [&](const coordinate &pos, Direction dir, int distance) {
        if (auto data = map.find(pos); data != map.end() && distance >= data->second.distance) {
            return;
        }
        auto status = moveRobot(dir);
        map[pos] = { status, ++distance };
        if (status == Status::Wall) {
            return;
        }
        for (auto newDir : directions) {
            explore(move(pos, newDir), newDir, distance);
        }
        moveRobot(opposite(dir));
    };

    for (auto dir : directions) {
        explore(move(robotPos, dir), dir, 0);
    }

    auto oxygenSystem = std::find_if(map.begin(), map.end(), [](const auto &a) {
        return a.second.type == Status::OxygenSystem;
    });

    // Begin Part B here

    // BFS implementation from oxygen system location
    std::map<coordinate, int> visited;
    std::list<std::pair<coordinate, int>> toVisit { { oxygenSystem->first, 0 } };
    while (!toVisit.empty()) {
        auto [pos, dist] = toVisit.front();
        toVisit.pop_front();
        // Already visited this location before
        if (auto data = visited.find(pos); data != visited.end()) {
            continue;
        }
        // No need to include walls
        if (map[pos].type == Status::Wall) {
            continue;
        }
        // Update visited map with distance and add all possible directions to queue
        visited[pos] = dist;
        for (auto dir : directions) {
            toVisit.push_back({ move(pos, dir), dist + 1 });
        }
    }

    auto farthest = std::max_element(visited.begin(), visited.end(), [](const auto &a, const auto &b) {
        return a.second < b.second;
    });

    return farthest->second;
}