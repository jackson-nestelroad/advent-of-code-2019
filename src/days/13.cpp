#include "../AdventOfCode2019.hpp"

enum class TileId {
    Empty = 0,
    Wall = 1,
    Block = 2,
    HorizontalPaddle = 3,
    Ball = 4
};

enum class Joystick {
    Neutral = 0,
    Left = -1,
    Right = 1
};

struct tile {
    IntCode::Int right;
    IntCode::Int top;
    TileId id;
};

IntCode::Computer &operator>>(IntCode::Computer &comp, tile &val) {
    comp >> val.right;
    comp >> val.top;
    IntCode::Int id;
    comp >> id;
    val.id = static_cast<TileId>(id);
    return comp;
}

int AoC::A::day13() {
    auto input = in::readFile<in::value<IntCode::Int, ','>>("input/13.txt");
    IntCode::Computer comp { input };
    comp.run();
    tile next { };
    int count = 0;
    while (comp.hasOutput()) {
        comp >> next;
        if (next.id == TileId::Block) {
            ++count;
        }
    }
    return count;
}

int AoC::B::day13() {
    auto input = in::readFile<in::value<IntCode::Int, ','>>("input/13.txt");
    IntCode::Computer comp { input };
    comp[0] = 2;

    tile next { };
    tile ball { };
    tile paddle { };
    int score = 0;
    while (!comp.isFinished()) {
        comp.run();
        while (comp.hasOutput()) {
            comp >> next;
            if (next.id == TileId::Ball) {
                ball = next;
            }
            else if (next.id == TileId::HorizontalPaddle) {
                paddle = next;
            }
            else if (next.right == -1 && next.top == 0) {
                score = static_cast<int>(next.id);
            }
        }
        comp << Templates::compare(ball.right, paddle.right);
    }
    return score;
}