#include "../AdventOfCode2019.hpp"

namespace IntCode {
    using Integers = std::vector<int>;

    enum Instructions {
        Add = 1,
        Multiply = 2,
        Exit = 99
    };

    class Computer {
    private:
        Integers data;
        bool running = false;

    public:
        Computer(const Integers &data) : data(data) { }

        void run() {
            int ip = 0;
            running = true;
            while (running && ip < data.size()) {
                switch (data[ip]) {
                    case Add: data[data[ip + 3]] = data[data[ip + 1]] + data[data[ip + 2]]; ip += 4; break;
                    case Multiply: data[data[ip + 3]] = data[data[ip + 1]] * data[data[ip + 2]]; ip += 4; break;
                    case Exit: running = false; break;
                    default: console::fatal("Invalid opcode at position ", ip, ".");
                }
            }
            running = false;
        }

        int &operator[](int i) {
            return data[i];
        }
    };
}

int AoC::A::day02() {
    auto input = in::readFile<in::value<int, ','>>("input/02.txt");
    IntCode::Computer comp { input };

    comp[1] = 12;
    comp[2] = 2;

    comp.run();

    return comp[0];
}

int AoC::B::day02() {
    auto input = in::readFile<in::value<int, ','>>("input/02.txt");

    for (int noun = 0; noun <= 99; ++noun) {
        input[1] = noun;
        for (int verb = 0; verb <= 99; ++verb) {
            input[2] = verb;
            IntCode::Computer comp { std::vector(input) };
            comp.run();
            if (comp[0] == 19690720) {
                return 100 * noun + verb;
            }
        }
    }
    return -1;
}