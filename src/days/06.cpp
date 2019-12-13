#include "../AdventOfCode2019.hpp"

using id = std::string;
using orbits = std::list<id>;
using OrbitMap = std::map<id, orbits>;
using Path = std::unordered_map<id, int>;

struct planet {
    std::optional<id> parent;
    orbits children;
};

using Galaxy = std::map<id, planet>;

struct counter {
    int children = 0;
    int combinations = 0;
};

void countOrbits(const OrbitMap &relations, const id &id, counter &data) {
    auto children = relations.at(id);
    if (children.empty()) {
        return;
    }
    if (children.size() == 1) {
        countOrbits(relations, children.front(), data);
        data.children += 1;
        data.combinations += data.children;
    }
    else {
        for (auto child : children) {
            counter count;
            countOrbits(relations, child, count);
            data.children += count.children + 1;
            data.combinations += count.children + count.combinations + 1;
        }
    }
}

int countOrbits(const OrbitMap &relations, const id &id) {
    counter res;
    countOrbits(relations, id, res);
    return res.combinations;
}

Path pathToRoot(const Galaxy &system, const id &start) {
    Path path;
    planet next = system.at(start);
    int i = 0;
    while (next.parent.has_value()) {
        id parent = next.parent.value();
        path.insert({ parent, i++ });
        next = system.at(parent);
    }
    return path;
}

int AoC::A::day06() {
    auto input = in::readFile<in::line<>, in::line<id, ')'>>("input/06.txt");
    OrbitMap orbits;
    for (auto direct : input) {
        if (orbits.find(direct[0]) == orbits.end()) {
            orbits.insert({ direct[0], { direct[1] } });
        }
        else {
            orbits.at(direct[0]).push_back(direct[1]);
        }
        if (orbits.find(direct[1]) == orbits.end()) {
            orbits.insert({ direct[1], { } });
        }
    }
    
    return countOrbits(orbits, "COM");
}

int AoC::B::day06() {
    auto input = in::readFile<in::line<>, in::line<id, ')'>>("input/06.txt");

    // Now need to record parents as well to travel upwards
    Galaxy system;
    for (auto direct : input) {
        if (system.find(direct[0]) == system.end()) {
            system[direct[0]] = { { }, { direct[1] } };
        }
        else {
            system.at(direct[0]).children.push_back(direct[1]);
        }
        if (system.find(direct[1]) == system.end()) {
            system[direct[1]] = { direct[0] };
        }
        else {
            system.at(direct[1]).parent = direct[0];
        }
    }

    // Get full path to root from YOU
    auto myPath = pathToRoot(system, "YOU");

    // Trace path from SAN to root, but only need to record distance travelled to lowest common ancestor
    planet next = system.at("SAN");
    int distToSanta = 0;
    while (next.parent.has_value()) {
        id parent = next.parent.value();
        if (auto dist = myPath.find(parent); dist != myPath.end()) {
            return dist->second + distToSanta;
        }
        ++distToSanta;
        next = system.at(parent);
    }
    console::fatal("No common path found.");
    return 0;
}