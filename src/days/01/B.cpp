#include "../../AdventOfCode2019.hpp"

int fuelRequirement(int mass, int result = 0) {
    int fuel = (mass / 3) - 2;
    if (fuel <= 0) {
        return result;
    }
    return fuelRequirement(fuel, result + fuel);
}

int aoc::B::day01() {
    auto input = InputReader::readFile<int>("days/01/A.txt", [](auto str) -> int { return std::stoi(str); });

    std::transform(input.begin(), input.end(), input.begin(), [](auto mass) -> int { return fuelRequirement(mass); });

    return std::accumulate(input.begin(), input.end(), 0, std::plus<int>());
}
