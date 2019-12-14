#include "../AdventOfCode2019.hpp"

namespace IntCode3 {
    using Integers = std::vector<int>;
    using Buffer = std::list<int>;

    enum AddressMode {
        Position = 0,
        Immediate = 1
    };

    enum Instruction {
        Add = 1,
        Multiply = 2,
        Input = 3,
        Output = 4,
        JumpIfTrue = 5,
        JumpIfFalse = 6,
        LessThan = 7,
        Equals = 8,
        Exit = 99
    };

    enum ProgramMode {
        Normal = 0,
        FeedbackLoop = 1
    };

    class Computer {
    private:
        int mode = 0;
        Integers data;
        std::size_t ip = 0;
        bool running = false;
        bool finished = false;

        Buffer in;
        Buffer out;

        static constexpr int POW10[7] = {
            1, 10, 100, 1'000, 10'000, 100'000, 1'000'000
        };

        inline int &param(const AddressMode mode, const int pos) {
            switch (mode) {
                case Position: return data[data[ip + pos]];
                case Immediate: return data[ip + pos];
                default: console::fatal("Unexpected address mode at position ", ip);
            }
            return data[ip];
        }

        inline int &calc(int modes, const int pos) {
            int paramMode = (modes / POW10[pos - 1]) % 10;
            return param(static_cast<AddressMode>(paramMode), pos);
        }

        void opcode(int instr) {
            int opcode = instr % 100;
            int modes = instr / 100;

            switch (opcode) {
                case Add: param(Position, 3) = calc(modes, 1) + calc(modes, 2); ip += 4; break;
                case Multiply: param(Position, 3) = calc(modes, 1) * calc(modes, 2); ip += 4; break;
                case Input: handleInput(); break;
                case Output: output(param(Position, 1)); ip += 2; break;
                case JumpIfTrue: calc(modes, 1) != 0 ? ip = calc(modes, 2) : ip += 3; break;
                case JumpIfFalse: calc(modes, 1) == 0 ? ip = calc(modes, 2) : ip += 3; break;
                case LessThan: param(Position, 3) = calc(modes, 1) < calc(modes, 2) ? 1 : 0; ip += 4; break;
                case Equals: param(Position, 3) = calc(modes, 1) == calc(modes, 2) ? 1 : 0; ip += 4; break;
                case Exit: running = false; finished = true; break;
                default: console::fatal("Unexpected opcode at position ", ip);
            }
        }

        inline int input() {
            int res = in.front();
            in.pop_front();
            return res;
        }

        inline void output(int val) {
            out.push_back(val);
        }

        inline void handleInput() {
            if (in.empty()) {
                switch (mode) {
                    case FeedbackLoop: running = false; return;
                    default: console::fatal("Input instruction met with no input.");
                }
            }
            param(Position, 1) = input();
            ip += 2;
        }

    public:
        Computer(const Integers &data) : Computer(data, Normal) { }
        Computer(const Integers &data, const ProgramMode &mode) : data(data), mode(mode) { }

        void run() {
            running = true;
            while (running && ip < data.size()) {
                opcode(data[ip]);
            }
            running = false;
        }

        void reset() {
            ip = 0;
            running = false;
            finished = false;
            in.clear();
            out.clear();
            data.clear();
        }

        void load(const Integers &program) {
            data = program;
        }
        
        bool isRunning() {
            return running;
        }

        bool isFinished() {
            return finished;
        }

        int &operator[](int i) {
            return data[i];
        }

        Computer &operator<<(int val) {
            in.push_back(val);
            return *this;
        }

        Computer &operator>>(int &val) {
            val = out.front();
            out.pop_front();
            return *this;
        }

        Buffer flushOutput() {
            Buffer flushed;
            std::copy(out.begin(), out.end(), std::back_inserter(flushed));
            out.clear();
            return flushed;
        }
    };
}

int AoC::A::day07() {
    auto input = in::readFile<in::value<int, ','>>("input/07.txt");
    
    constexpr int numAmps = 5;
    std::vector<int> thrusters;
    std::vector<int> phase = { 0, 1, 2, 3, 4 };
    std::sort(phase.begin(), phase.end());
    do {
        IntCode3::Computer amps[] { input, input, input, input, input };

        int last = 0;
        for (int i = 0; i < numAmps; ++i) {
            amps[i] << phase[i] << last;
            amps[i].run();
            amps[i] >> last;
        }
        thrusters.push_back(last);
    } while (std::next_permutation(phase.begin(), phase.end()));

    auto max = std::max_element(thrusters.begin(), thrusters.end());

    return *max;
}

int AoC::B::day07() {
    auto input = in::readFile<in::value<int, ','>>("input/07.txt");

    constexpr int numAmps = 5;

    std::vector<int> thrusters;
    std::vector<int> phase = { 5, 6, 7, 8, 9 };
    std::sort(phase.begin(), phase.end());
    do {
        std::vector<IntCode3::Computer> amps;
        amps.assign(numAmps, IntCode3::Computer { input, IntCode3::FeedbackLoop });
        for (int i = 0; i < numAmps; ++i) {
            amps[i] << phase[i];
        }

        int last = 0;
        for (int i = 0; !amps[numAmps - 1].isFinished(); i = ++i % numAmps) {
            amps[i] << last;
            amps[i].run();
            amps[i] >> last;
        }
        thrusters.push_back(last);
    } while (std::next_permutation(phase.begin(), phase.end()));

    auto max = std::max_element(thrusters.begin(), thrusters.end());

    return *max;
}