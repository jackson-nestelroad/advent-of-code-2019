#include "../AdventOfCode2019.hpp"

enum class DroneStatus {
    Stationary = 0,
    Pulled = 1
};

int AoC::A::day19() {
    auto input = in::readFile<in::value<IntCode::Int, ','>>("input/19.txt");
    IntCode::Computer comp;
    constexpr std::size_t size = 50;

    int count = 0;
    int x = 0;
    int y = 0;
    int firstX = -1;
    while (x < size && y < size) {
        comp.load(input);
        comp << x << y;
        comp.run();
        switch (static_cast<DroneStatus>(comp.popLastOutput())) {
            case DroneStatus::Pulled:
                ++count;
                if (firstX < 0) {
                    firstX = x;
                }
                ++x;
                break;
            case DroneStatus::Stationary:
                if (firstX < 0) {
                    ++x;
                    if (x >= size) {
                        x = 0;
                        ++y;
                    }
                }
                else {
                    ++y;
                    x = firstX;
                    firstX = -1;
                }
                break;
        }
    }
    return count;
}

int AoC::B::day19() {
    auto input = in::readFile<in::value<IntCode::Int, ','>>("input/19.txt");
    IntCode::Computer comp;

    constexpr std::size_t shipSize = 100;
    constexpr std::size_t shipSizeMinusOne = shipSize - 1;

    std::vector<int> rowFirst;
    std::vector<int> rowWidth;

    const auto getStatus = [&comp, &input = std::as_const(input)](int x, int y) {
        comp.load(input);
        comp << x << y;
        comp.run();
        return static_cast<DroneStatus>(comp.popLastOutput());
    };

    int x = 0;
    int y = 0;
    int firstX = -1;
    for (y; ; x = firstX, firstX = -1, ++y) {
        for (x; ;) {
            if (getStatus(x, y) == DroneStatus::Pulled) {
                // This is the beginning of the beam
                if (firstX < 0) {
                    firstX = x;
                    // Jump forward according to previous row's width
                    x += y > 0 ? rowWidth[y - 1] - 1 : 1;
                }
                // Move forward one at a time to find the end of the beam
                else {
                    ++x;
                }
            }
            // May be before beginning of beam or after end of beam
            else {
                if (firstX < 0) {
                    // Handles edge case where row has no parts of the beam in it
                    if (x++ > rowFirst[y - 1] + rowWidth[y - 1]) {
                        firstX = rowFirst[y - 1];
                        rowWidth.push_back(rowWidth[y - 1]);
                        rowFirst.push_back(firstX);
                        break;
                    }
                }
                // Found end of beam
                else {
                    int width = x - firstX;
                    rowWidth.push_back(width);
                    rowFirst.push_back(firstX);
                    
                    // std::cout << std::string(firstX, '.') << std::string(width, '#') << std::endl;

                    // Check if we can create square of shipSize
                    if (width >= shipSize) {
                        // Top of the square (y-coordinate of top)
                        int topRow = y - shipSizeMinusOne;
                        // Top of the square can fit the square
                        if (int topRowEnd = rowFirst[topRow] + rowWidth[topRow] - 1; topRowEnd >= firstX + shipSizeMinusOne) {
                            // x-coordinate of top
                            int topRowBegin = topRowEnd - shipSizeMinusOne;
                            // (topRowBegin, topRow) is top-left corner of square and is closest to the emitter
                            return topRowBegin * 10'000 + topRow;
                        }
                    }
                    // Move to the next row
                    break;
                }
            }
        }
    }
}