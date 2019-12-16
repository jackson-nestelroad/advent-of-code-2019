#include "../AdventOfCode2019.hpp"

struct vector {
    int x;
    int y;
    int z;
};

vector &operator+=(vector &a, const vector &b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

int compare(int a, int b) {
    if (a < b) {
        return -1;
    }
    else if (a > b) {
        return 1;
    }
    return 0;
}

vector gravity(const vector &a, const vector &b) {
    return { compare(b.x, a.x), compare(b.y, a.y), compare(b.z, a.z) };
}

struct moon {
    vector position { 0, 0, 0 };
    vector velocity { 0, 0, 0 };

    void step() {
        position += velocity;
    }

    void applyGravity(const moon &other) {
        velocity += gravity(position, other.position);
    }

    int kineticEnergy() const {
        return std::abs(velocity.x) + std::abs(velocity.y) + std::abs(velocity.z);
    }

    int potentialEnergy() const {
        return std::abs(position.x) + std::abs(position.y) + std::abs(position.z);
    }

    int totalEnergy() const {
        return kineticEnergy() * potentialEnergy();
    }
};

std::istream &operator>>(std::istream &input, moon &val) {
    int parsed[3];
    int current = 0;
    while (current < 3) {
        if (!(input >> parsed[current])) {
            input.clear();
            input.ignore();
            if (input.peek() == EOF) {
                return input;
            }
        }
        else {
            ++current;
        }
    }
    val.position.x = parsed[0];
    val.position.y = parsed[1];
    val.position.z = parsed[2];
    return input;
}

int AoC::A::day12() {
    auto moons = in::readFile<in::value<moon, '\n'>>("input/12.txt");
    constexpr int steps = 1000;
    for (int i = 0; i < steps; ++i) {
        for (auto &moon : moons) {
            for (auto &other : moons) {
                moon.applyGravity(other);
            }
        }
        for (auto &moon : moons) {
            moon.step();
        }
    }
    int totalEnergy = std::accumulate(moons.begin(), moons.end(), 0, [](int sum, const moon &a) {
        return sum + a.totalEnergy();
    });
    return totalEnergy;
}

using OneDimensionalState = std::vector<std::pair<int, int>>;

long long getCycle(OneDimensionalState state) {
    auto initialState = state;
    const std::size_t size = state.size();
    long long cycles = 0;
    do {
        for (int i = 0; i < size; ++i) {
            for (int j = i + 1; j < size; ++j) {
                int delta = compare(state[j].first, state[i].first);
                state[i].second += delta;
                state[j].second -= delta;
            }
        }
        for (auto &state : state) {
            state.first += state.second;
        }
        ++cycles;
    } while (state != initialState);
    return cycles;
}

long long AoC::B::day12() {
    auto moons = in::readFile<in::value<moon, '\n'>>("input/12.txt");

    OneDimensionalState xState, yState, zState;
    std::transform(moons.begin(), moons.end(), std::back_inserter(xState), [](const moon &a) -> std::pair<int, int> {
        return { a.position.x, a.velocity.x };
    });
    std::transform(moons.begin(), moons.end(), std::back_inserter(yState), [](const moon &a) -> std::pair<int, int> {
        return { a.position.y, a.velocity.y };
    });
    std::transform(moons.begin(), moons.end(), std::back_inserter(zState), [](const moon &a) -> std::pair<int, int> {
        return { a.position.z, a.velocity.z };
    });

    // Run all dimensions at once
    auto xCycle = std::async(std::launch::async, getCycle, xState);
    auto yCycle = std::async(std::launch::async, getCycle, yState);
    auto zCycle = std::async(std::launch::async, getCycle, zState);

    auto x = xCycle.get();
    auto y = yCycle.get();
    auto z = zCycle.get();

    // Find where all cycles match up to bring moons to original state
    return Integer::lcm(x, y, z);
}