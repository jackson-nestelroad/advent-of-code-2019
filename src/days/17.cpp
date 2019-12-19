#include "../AdventOfCode2019.hpp"

enum class Robot {
    Left = '<',
    Right = '>',
    Up = '^',
    Down = 'v',
    Tumbling = 'X'
};

enum class Grid {
    Scaffold = '#',
    Space = '.'
};

std::vector<std::vector<IntCode::Int>> getCameraData(IntCode::Computer &comp) {
    std::vector<std::vector<IntCode::Int>> camera { { } };
    auto feed = comp.flushOutput();
    int row = 0;
    for (auto i : feed) {
        if (i == 10) {
            camera.push_back({ });
            ++row;
        }
        else {
            camera[row].push_back(i);
        }
    }
    camera.erase(std::remove_if(camera.begin(), camera.end(), [](const auto &a) { return a.empty(); }), camera.end());
    return camera;
}

int AoC::A::day17() {
    auto input = in::readFile<in::value<IntCode::Int, ','>>("input/17.txt");
    IntCode::Computer comp { input };
    comp.run();
    auto grid = getCameraData(comp);
    const std::size_t height = grid.size();
    const std::size_t width = grid[0].size();
    int result = 0;
    for (int row = 1; row < height - 1; ++row) {
        for (int col = 1; col < width - 1; ++col) {
            if (grid[row][col] == '#') {
                if (grid[row - 1][col] == '#' && grid[row + 1][col] == '#'
                    && grid[row][col - 1] == '#' && grid[row][col + 1] == '#') {
                    result += row * col;
                }
            }
        }
    }
    return result;
}

std::pair<int, int> nextPosition(const std::pair<int, int> &pos, Robot dir) {
    auto res = pos;
    switch (dir) {
        case Robot::Up: res.second -= 1; break;
        case Robot::Right: res.first += 1; break;
        case Robot::Down: res.second += 1; break;
        case Robot::Left: res.first -= 1; break;
    }
    return res;
}

Robot turnLeft(Robot dir) {
    switch (dir) {
        case Robot::Up: return Robot::Left;
        case Robot::Right: return Robot::Up;
        case Robot::Down: return Robot::Right;
        case Robot::Left: return Robot::Down;
    }
    return Robot::Up;
}

Robot turnRight(Robot dir) {
    switch (dir) {
        case Robot::Up: return Robot::Right;
        case Robot::Right: return Robot::Down;
        case Robot::Down: return Robot::Left;
        case Robot::Left: return Robot::Up;
    }
    return Robot::Up;
}

int AoC::B::day17() {
    auto input = in::readFile<in::value<IntCode::Int, ','>>("input/17.txt");
    IntCode::Computer comp { input };
    comp.run();
    auto grid = getCameraData(comp);
    const int height = static_cast<int>(grid.size());
    const int width = static_cast<int>(grid[0].size());

    // Find robot in grid
    std::pair<int, int> pos;
    Robot dir;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            auto curr = grid[y][x];
            if (curr != '#' && curr != '.') {
                pos = { x, y };
                dir = static_cast<Robot>(curr);
            }
        }
    }

    // Create path the robot must take throuh the scaffolding
    std::string path;
    const auto get = [&](const std::pair<int, int> &pos) {
        if (pos.first < 0 || pos.first >= width || pos.second < 0 || pos.second >= height) {
            return Grid::Space;
        }
        return static_cast<Grid>(grid[pos.second][pos.first]);
    };

    const auto addToPath = [&](int steps) {
        if (steps != 0) {
            path += std::to_string(steps);
            path.push_back(',');
        }
    };

    int steps = 0;
    while (true) {
        auto nextPos = nextPosition(pos, dir);
        auto nextChar = get(nextPos);
        // Move straight as far as possible
        if (nextChar == Grid::Scaffold) {
            ++steps;
            pos = nextPos;
        }
        else {
            // Try turning left first
            auto leftDir = turnLeft(dir);
            auto leftPos = nextPosition(pos, leftDir);
            auto leftChar = get(leftPos);
            if (leftChar == Grid::Scaffold) {
                addToPath(steps);
                steps = 1;
                path.push_back('L');
                path.push_back(',');
                pos = leftPos;
                dir = leftDir;
            }
            else {
                // Try turning right
                auto rightDir = turnRight(dir);
                auto rightPos = nextPosition(pos, rightDir);
                auto rightChar = get(rightPos);
                if (rightChar == Grid::Scaffold) {
                    addToPath(steps);
                    steps = 1;
                    path.push_back('R');
                    path.push_back(',');
                    pos = rightPos;
                    dir = rightDir;
                }
                else {
                    // We have reached the end of the scaffolding, path is complete
                    addToPath(steps);
                    break;
                }
            }
        }
    }

    // Originally I was going to opt for writing a string pattern finder, 
    // but this was going to be a lot more work for this narrow problem scope,
    // so this RegEx finds our precise patterns using back references
    std::regex compresser { "^(.{1,21})\\1*(.{1,21})(?:\\1|\\2)*(.{1,21})(?:\\1|\\2|\\3)*$" };
    std::smatch match;
    auto foundRoutines = std::regex_search(path, match, compresser);
    if (!foundRoutines) {
        console::fatal("Path could not be compressed.");
    }
    // Store routines
    std::map<char, std::string> routines {
        { 'A', match[1] },
        { 'B', match[2] },
        { 'C', match[3] }
    };
    
    // Build the main routine by finding all subroutines in the path
    std::map<std::size_t, char> mainRoutine;
    for (const auto &[id, subPath] : routines) {
        std::size_t pos = path.find(subPath);
        while (pos != std::string::npos) {
            mainRoutine.insert({ pos, id });
            pos = path.find(subPath, pos + 1);
        }
    }

    // Reset the IntCode computer
    comp.reset();
    comp.load(input);

    // Input main routine
    for (auto it = mainRoutine.begin(); it != mainRoutine.end(); ++it) {
        comp << it->second;
        if (std::distance(it, mainRoutine.end()) > 1) {
            comp << ',';
        }
    }
    comp << '\n';
    
    // Input subroutines
    for (const auto &[id, subPath] : routines) {
        const int length = static_cast<int>(subPath.length() - 1);
        for (int i = 0; i < length; ++i) {
            comp << subPath[i];
        }
        comp << '\n';
    }
    // No camera output wanted
    comp << 'n' << '\n';

    // Run the routine
    comp[0] = 2;
    comp.run();
    return static_cast<int>(comp.flushOutput().back());
}