#include "../../AdventOfCode2019.hpp"

int aoc::A::day01() {
    auto input = InputReader::readFile<int>("days/01/A.txt", [](auto str) -> int { return std::stoi(str); });

    std::transform(input.begin(), input.end(), input.begin(), [](auto mass) -> int {
        return (mass / 3) - 2;
    });

    return std::accumulate(input.begin(), input.end(), 0, std::plus<int>());
}