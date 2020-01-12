#pragma once
#include <vector>
#include <list>

#include "Console.hpp"

namespace IntCode {
    using Int = long long;
    using Integers = std::vector<Int>;
    using Buffer = std::list<Int>;

    enum class AddressMode {
        Position = 0,
        Immediate = 1,
        Relative = 2
    };

    enum class Instruction {
        Add = 1,
        Multiply = 2,
        Input = 3,
        Output = 4,
        JumpIfTrue = 5,
        JumpIfFalse = 6,
        LessThan = 7,
        Equals = 8,
        AdjustRelativeBase = 9,
        Exit = 99
    };

    enum class Status {
        Good,
        AwaitingInput
    };

    enum class Output {
        Int,
        ASCII
    };

    class Computer {
    private:
        Status status = Status::Good;
        Integers data;
        std::size_t ip = 0;
        std::size_t rb = 0;
        bool running = false;
        bool finished = false;
        Output outputMode = Output::Int;

        Buffer in;
        Buffer out;

        static constexpr int POW10[7] = {
            1, 10, 100, 1'000, 10'000, 100'000, 1'000'000
        };

        Int read(const Int modes, const Int pos) {
            int paramMode = (modes / POW10[pos - 1]) % 10;
            std::size_t address { };
            switch (static_cast<AddressMode>(paramMode)) {
                case AddressMode::Position: address = data[ip + pos]; break;
                case AddressMode::Immediate: address = ip + pos; break;
                case AddressMode::Relative: address = rb + data[ip + pos]; break;
                default: console::fatal("Unexpected address mode at position ", ip);
            }
            if (address >= data.size()) {
                data.resize(address + 1);
            }
            return data[address];
        }

        Int &write(const Int modes, const Int pos) {
            int paramMode = (modes / POW10[pos - 1]) % 10;
            std::size_t address { };
            switch (static_cast<AddressMode>(paramMode)) {
                case AddressMode::Position: address = data[ip + pos]; break;
                case AddressMode::Relative: address = rb + data[ip + pos]; break;
                default: console::fatal("Unexpected address mode at position ", ip);
            }
            if (address >= data.size()) {
                data.resize(address + 1);
            }
            return data[address];
        }

        void opcode(Int instr) {
            Int opcode = instr % 100;
            Int modes = instr / 100;

            switch (static_cast<Instruction>(opcode)) {
                case Instruction::Add: write(modes, 3) = read(modes, 1) + read(modes, 2); ip += 4; break;
                case Instruction::Multiply: write(modes, 3) = read(modes, 1) * read(modes, 2); ip += 4; break;
                case Instruction::Input: handleInput(opcode, modes); break;
                case Instruction::Output: output(read(modes, 1)); ip += 2; break;
                case Instruction::JumpIfTrue: read(modes, 1) != 0 ? ip = read(modes, 2) : ip += 3; break;
                case Instruction::JumpIfFalse: read(modes, 1) == 0 ? ip = read(modes, 2) : ip += 3; break;
                case Instruction::LessThan: write(modes, 3) = read(modes, 1) < read(modes, 2) ? 1 : 0; ip += 4; break;
                case Instruction::Equals: write(modes, 3) = read(modes, 1) == read(modes, 2) ? 1 : 0; ip += 4; break;
                case Instruction::AdjustRelativeBase: rb += read(modes, 1); ip += 2; break;
                case Instruction::Exit: running = false; finished = true; break;
                default: console::fatal("Unexpected opcode at position ", ip);
            }
        }

        inline Int input() {
            Int res = in.front();
            in.pop_front();
            return res;
        }

        inline void output(Int val) {
            out.push_back(val);
        }

        void handleInput(Int opcode, Int modes) {
            if (in.empty()) {
                status = Status::AwaitingInput;
                running = false;
            }
            else {
                write(modes, 1) = input();
                ip += 2;
            }
        }

    public:
        Computer() : data() { }
        Computer(const Integers &data) : data(data) { }

        void run() {
            status = Status::Good;
            running = true;
            while (running && ip < data.size()) {
                opcode(data[ip]);
            }
            running = false;
        }

        void reset() {
            ip = 0;
            rb = 0;
            status = Status::Good;
            running = false;
            finished = false;
            in.clear();
            out.clear();
            data.clear();
        }

        void load(const Integers &program) {
            reset();
            data = program;
        }

        bool isRunning() const {
            return running;
        }

        bool isFinished() const {
            return finished;
        }

        bool hasInput() const {
            return !in.empty();
        }

        bool hasOutput() const {
            return !out.empty();
        }

        Status getStatus() const {
            return status;
        }

        Int &operator[](Int i) {
            return data[i];
        }

        Computer &operator<<(Output flag) {
            outputMode = flag;
            return *this;
        }

        Computer &operator<<(Int val) {
            in.push_back(val);
            return *this;
        }

        Computer &operator>>(Int &val) {
            val = out.front();
            out.pop_front();
            return *this;
        }

        friend std::ostream &operator<<(std::ostream &out, Computer &comp) {
            switch (comp.outputMode) {
                case Output::ASCII:
                    for (auto val : comp.out) {
                        out << static_cast<char>(val);
                    }
                    break;
                default:
                    for (auto val : comp.out) {
                        out << val << std::endl;
                    }
                    break;
            }
            comp.out.clear();
            return out;
        }

        Buffer flushOutput() {
            Buffer flushed;
            std::copy(out.begin(), out.end(), std::back_inserter(flushed));
            out.clear();
            return flushed;
        }

        Int popLastOutput() {
            Int res = out.front();
            out.pop_front();
            return res;
        }
    };
}