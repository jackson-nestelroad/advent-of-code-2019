#include "../AdventOfCode2019.hpp"

inline int fuelRequirement(int mass) {
    return (mass / 3) - 2;
}

int fuelRequirementRecursive(int mass, int result = 0) {
    int fuel = fuelRequirement(mass);
    if (fuel <= 0) {
        return result;
    }
    return fuelRequirementRecursive(fuel, result + fuel);
}

int AoC::A::day01() {
    auto input = in::readFile<in::value<int, '\n'>>("input/01.txt");

    std::transform(input.begin(), input.end(), input.begin(), fuelRequirement);

    return std::accumulate(input.begin(), input.end(), 0, std::plus<int>());
}

int AoC::B::day01() {
    auto input = in::readFile<in::value<int, '\n'>>("input/01.txt");

    std::transform(input.begin(), input.end(), input.begin(), [](auto mass) { return fuelRequirementRecursive(mass); });

    return std::accumulate(input.begin(), input.end(), 0, std::plus<int>());
}