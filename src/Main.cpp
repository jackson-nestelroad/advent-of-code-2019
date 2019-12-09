#include <iostream>
#include <chrono>

#include "AdventOfCode2019.hpp"

int main() {
	auto start = std::chrono::system_clock::now().time_since_epoch();

	console::string("Solution: ", aoc::B::day01());
	
	auto end = std::chrono::system_clock::now().time_since_epoch();

	auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	console::string("Time: ", elapsedTime.count(), " ms");

	return 0;
}