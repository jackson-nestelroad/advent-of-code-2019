#include "../AdventOfCode2019.hpp"

#define POW10(n) 

namespace IntCode2 {
    using Integers = std::vector<int>;
    using Buffer = std::list<int>;

    enum Modes {
        Position = 0,
        Immediate = 1
    };

    enum Instructions {
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

    class Computer {
    private:
        Integers data;
        bool running = false;
        std::size_t ip;
        
        Buffer input;
        Buffer output;

        static constexpr int POW10[7] = {
            1, 10, 100, 1'000, 10'000, 100'000, 1'000'000
        };

        inline int &mode(const int mode, const int pos) {
            switch (mode) {
                case Position: return data[data[ip + pos]];
                case Immediate: return data[ip + pos];
                default: console::fatal("Unexpected address mode at position ", ip);
            }
        }

        inline int &param(int modes, const int pos) {
            int paramMode = (modes / POW10[pos - 1]) % 10;
            return mode(paramMode, pos);
        }

        void handleOpcode(int instr) {
            int opcode = instr % 100;
            int modes = instr / 100;

            switch (opcode) {
                case Add: mode(Position, 3) = param(modes, 1) + param(modes, 2); ip += 4; break;
                case Multiply: mode(Position, 3) = param(modes, 1) * param(modes, 2); ip += 4; break;
                case Input: mode(Position, 1) = input.front(); input.pop_front();  ip += 2; break;
                case Output: output.push_back(mode(Position, 1)); ip += 2; break;
                case Exit: running = false; break;
                default: console::fatal("Unexpected opcode at position ", ip);
            }
        }

        void handleOpcodeExtended(int instr) {
            int opcode = instr % 100;
            int modes = instr / 100;

            switch (opcode) {
                case Add: mode(Position, 3) = param(modes, 1) + param(modes, 2); ip += 4; break;
                case Multiply: mode(Position, 3) = param(modes, 1) * param(modes, 2); ip += 4; break;
                case Input: mode(Position, 1) = input.front(); input.pop_front();  ip += 2; break;
                case Output: output.push_back(mode(Position, 1)); ip += 2; break;
                case JumpIfTrue: param(modes, 1) != 0 ? ip = param(modes, 2) : ip += 3; break;
                case JumpIfFalse: param(modes, 1) == 0 ? ip = param(modes, 2) : ip += 3; break;
                case LessThan: mode(Position, 3) = param(modes, 1) < param(modes, 2) ? 1 : 0; ip += 4; break;
                case Equals: mode(Position, 3) = param(modes, 1) == param(modes, 2) ? 1 : 0; ip += 4; break;
                case Exit: running = false; break;
                default: console::fatal("Unexpected opcode at position ", ip);
            }
        }

    public:
        Computer(const Integers &data) : data(data) { }

        void run() {
            ip = 0;
            running = true;
            while (running && ip < data.size()) {
                handleOpcode(data[ip]);
            }
            running = false;
        }

        void runExtended() {
            ip = 0;
            running = true;
            while (running && ip < data.size()) {
                handleOpcodeExtended(data[ip]);
            }
            running = false;
        }

        int &operator[](int i) {
            return data[i];
        }

        Computer &operator<<(int in) {
            input.push_back(in);
            return *this;
        }

        void log() {
            for (int i : output) {
                console::log(i);
            }
            output.clear();
        }

        Buffer flushOutput() {
            Buffer out;
            std::copy(output.begin(), output.end(), std::inserter(out, out.begin()));
            output.clear();
            return out;
        }
    };
}

int AoC::A::day05() {
    auto input = in::readFile<in::value<int, ','>>("input/05.txt");
    
    IntCode2::Computer comp { input };
    comp << 1;
    comp.run();
    
    auto output = comp.flushOutput();
    return output.back();
}

int AoC::B::day05() {
    auto input = in::readFile<in::value<int, ','>>("input/05.txt");

    IntCode2::Computer comp { input };
    comp << 5;
    comp.runExtended();

    auto output = comp.flushOutput();
    return output.front();
}