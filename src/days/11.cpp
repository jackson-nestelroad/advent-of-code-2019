#include "../AdventOfCode2019.hpp"

using coordinate = std::pair<int, int>;

enum class Panel {
    Black = 0,
    White = 1
};

enum class Turn {
    Left = 0,
    Right = 1
};

enum class Direction {
    Up = 0,
    Right = 1,
    Down = 2,
    Left = 3
};

struct PaintingRobot {
    Direction dir;
    coordinate pos;

    void turn(Turn val) {
        switch(val) {
            case Turn::Left:
                switch (dir) {
                    case Direction::Up: dir = Direction::Left; break;
                    case Direction::Right: dir = Direction::Up; break;
                    case Direction::Down: dir = Direction::Right; break;
                    case Direction::Left: dir = Direction::Down; break;
                }
                break;
            case Turn::Right:
                switch (dir) {
                    case Direction::Up: dir = Direction::Right; break;
                    case Direction::Right: dir = Direction::Down; break;
                    case Direction::Down: dir = Direction::Left; break;
                    case Direction::Left: dir = Direction::Up; break;
                }
                break;
        }
    }

    void forward() {
        switch (dir) {
            case Direction::Up: pos.second -= 1; break;
            case Direction::Right: pos.first += 1; break;
            case Direction::Down: pos.second += 1; break;
            case Direction::Left: pos.first -= 1; break;
        }
    }
};

int AoC::A::day11() {
    auto input = in::readFile<in::value<IntCode::Int, ','>>("input/11.txt");
    
    IntCode::Computer comp { input };
    std::map<coordinate, Panel> panels;
    PaintingRobot robot { Direction::Up, { 0, 0 } };

    while (!comp.isFinished()) {
        comp << static_cast<int>(panels[robot.pos]);
        comp.run();
        panels[robot.pos] = static_cast<Panel>(comp.popLastOutput());
        robot.turn(static_cast<Turn>(comp.popLastOutput()));
        robot.forward();
    }

    return static_cast<int>(panels.size());
}

int AoC::B::day11() {
    auto input = in::readFile<in::value<IntCode::Int, ','>>("input/11.txt");

    IntCode::Computer comp { input };
    std::map<coordinate, Panel> panels;
    PaintingRobot robot { Direction::Up, { 0, 0 } };

    // Keep track of X and Y limits
    coordinate limitX { 0, 0 };
    coordinate limitY { 0, 0 };

    panels[robot.pos] = Panel::White;
    while (!comp.isFinished()) {
        comp << static_cast<int>(panels[robot.pos]);
        comp.run();
        panels[robot.pos] = static_cast<Panel>(comp.popLastOutput());
        robot.turn(static_cast<Turn>(comp.popLastOutput()));
        robot.forward();

        if (robot.pos.first < limitX.first) {
            limitX.first = robot.pos.first;
        }
        else if (robot.pos.first > limitX.second) {
            limitX.second = robot.pos.first;
        }
        if (robot.pos.second < limitY.first) {
            limitY.first = robot.pos.second;
        }
        else if (robot.pos.second > limitY.second) {
            limitY.second = robot.pos.second;
        }
    }

    // Create a vector grid to hold all painted and unpainted coordinates
    std::size_t height = limitY.second - limitY.first + 1;
    std::size_t width = limitX.second - limitX.first + 1;
    std::vector<std::vector<Panel>> grid;
    grid.resize(height);
    for (auto &row : grid) {
        row.resize(width);
    }
    
    // Move all painted panels to vector grid
    for (auto panel : panels) {
        grid[panel.first.second - limitY.first][panel.first.first - limitX.first] = panel.second;
    }
    
    // Output painted results to file
    auto output = std::ofstream { "output/11.txt" };
    if (!output) {
        console::fatal("Could not open output file.");
    }
    for (auto row : grid) {
        for (auto panel : row) {
            output << (panel == Panel::White ? '1' : ' ');
        }
        output << std::endl;
    }

    console::log("See output file.");
    return 0;
}