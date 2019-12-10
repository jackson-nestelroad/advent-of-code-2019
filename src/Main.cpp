#include <iostream>
#include <string>
#include <chrono>
#include <functional>

#include "AdventOfCode2019.hpp"

void run(const std::string &text, const std::function<int()> &solution) {
    auto start = std::chrono::system_clock::now().time_since_epoch();
    console::log(text, solution());
    auto end = std::chrono::system_clock::now().time_since_epoch();

    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    console::log("Time: ", elapsedTime.count(), " ms");
}

int main() {
    run("Part A: ", AoC::A::day02);
    run("Part B: ", AoC::B::day02);
    return 0;
}