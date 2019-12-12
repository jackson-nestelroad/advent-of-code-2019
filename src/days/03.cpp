#include "../AdventOfCode2019.hpp"

struct wire {
    char dir;
    int length;

    friend std::istream &operator>>(std::istream &input, wire &value) {
        input.get(value.dir);
        input >> value.length;
        return input;
    }
};

struct point {
    int x;
    int y;
    int steps;
};

struct segment {
    point begin;
    point end;
    int wire;
};

bool operator<(const point &a, const point &b) {
    return std::tie(a.x, a.y) < std::tie(b.x, b.y);
}

bool operator<(const segment &a, const segment &b) {
    return std::tie(a.begin, a.end, a.wire) < std::tie(b.begin, b.end, b.wire);
}

int distance(const point &a, const point &b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

bool between(int val, int a, int b) {
    return std::min(a, b) < val && val < std::max(a, b);
}

std::optional<point> intersects(const segment &a, const segment &b) {
    if (a.wire == b.wire) {
        return { };
    }
    if (between(a.begin.x, b.begin.x, b.end.x) && between(b.begin.y, a.begin.y, a.end.y)) {
        return point { a.begin.x, b.end.y };
    }
    if (between(b.begin.x, a.begin.x, a.end.x) && between(a.begin.y, b.begin.y, b.end.y)) {
        return point { b.begin.x, a.end.y };
    }
    return { };
}

template <int N>
std::pair<std::list<segment>, std::list<segment>> tracePath(const std::vector<wire> &path) {
    std::list<segment> v;
    std::list<segment> h;
    point curr { 0, 0 };
    point prev = curr;
    for (auto wire : path) {
        curr.steps += wire.length;
        switch (wire.dir) {
            case 'R': curr.x += wire.length; break;
            case 'L': curr.x -= wire.length; break;
            case 'U': curr.y += wire.length; break;
            case 'D': curr.y -= wire.length; break;
            default: console::fatal("Unknown direction detected."); break;
        }
        segment s = { prev, curr, N };
        // Let the first point be the left, top-most to allow for easier scanning
        if (curr < prev) {
            std::swap(s.begin, s.end);
        }
        switch (wire.dir) {
            case 'R':
            case 'L': h.push_back(s); break;
            case 'U':
            case 'D': v.push_back(s); break;
            default: console::fatal("Unknown direction detected."); break;
        }
        prev = curr;
    }
    v.sort();
    h.sort();
    return std::pair{ v, h };
}

int AoC::A::day03() {
    auto input = in::readFile<in::line<>, in::value<wire, ','>>("input/03.txt");
    constexpr point centralPort { 0, 0 };

    auto first = std::async(std::launch::async, tracePath<0>, input[0]);
    auto second = std::async(std::launch::async, tracePath<1>, input[1]);

    auto firstWire = first.get();
    auto secondWire = second.get();

    std::list<segment> vertical;
    std::list<segment> horizontal;

    std::merge(firstWire.first.begin(), firstWire.first.end(), 
        secondWire.first.begin(), secondWire.first.end(), 
        std::inserter(vertical, vertical.begin()));
    std::merge(firstWire.second.begin(), firstWire.second.end(), 
        secondWire.second.begin(), secondWire.second.end(), 
        std::inserter(horizontal, horizontal.begin()));

    std::list<segment> seen;
    std::list<point> intersections;

    // Vertical line scanning using the present vertical lines
    for(auto vert : vertical) {
        int x = vert.begin.x;
        segment s;
        // Consider all horizontal lines that begin before this vertical line
        while (!horizontal.empty() && (s = horizontal.front()).begin.x <= x) {
            seen.push_back(s);
            horizontal.pop_front();
        }
        // Iterate over each horizontal line that may interesect the current vertical line
        for (auto it = seen.begin(); it != seen.end();) {
            // Line ends before current line, delete it
            if (it->end.x < x) {
                seen.erase(it++);
            }
            else {
                // Check for interesection
                if (auto intersection = intersects(vert, *it)) {
                    intersections.push_back(intersection.value());
                }
                ++it;
            }
        }
    }

    intersections.sort([centralPort](const point &a, const point &b) {
        return distance(centralPort, a) < distance(centralPort, b);
    });

    return distance(centralPort, intersections.front());
}

int AoC::B::day03() {
    auto input = in::readFile<in::line<>, in::value<wire, ','>>("input/03.txt");

    auto first = std::async(std::launch::async, tracePath<0>, input[0]);
    auto second = std::async(std::launch::async, tracePath<1>, input[1]);

    auto firstWire = first.get();
    auto secondWire = second.get();

    std::list<segment> vertical;
    std::list<segment> horizontal;

    std::merge(firstWire.first.begin(), firstWire.first.end(),
        secondWire.first.begin(), secondWire.first.end(),
        std::inserter(vertical, vertical.begin()));
    std::merge(firstWire.second.begin(), firstWire.second.end(),
        secondWire.second.begin(), secondWire.second.end(),
        std::inserter(horizontal, horizontal.begin()));

    std::list<segment> seen;
    std::list<point> intersections;

    for (auto vert : vertical) {
        int x = vert.begin.x;
        segment s;
        while (!horizontal.empty() && (s = horizontal.front()).begin.x <= x) {
            seen.push_back(s);
            horizontal.pop_front();
        }
        for (auto it = seen.begin(); it != seen.end();) {
            if (it->end.x < x) {
                seen.erase(it++);
            }
            else {
                if (auto intersection = intersects(vert, *it)) {
                    point p = intersection.value();

                    // Calculate additional steps from both wires
                    point begin = it->begin.steps < it->end.steps ? it->begin : it->end;
                    p.steps += begin.steps + std::abs(begin.x - p.x);

                    begin = vert.begin.steps < vert.end.steps ? vert.begin : vert.end;
                    p.steps += begin.steps + std::abs(begin.y - p.y);

                    intersections.push_back(p);
                }
                ++it;
            }
        }
    }

    intersections.sort([](const point &a, const point &b) {
        return a.steps < b.steps;
    });

    return intersections.front().steps;
}