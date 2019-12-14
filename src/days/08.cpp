#include "../AdventOfCode2019.hpp"

enum Colors {
    Black = '0',
    White = '1',
    Transparent = '2'
};

template <int H, int W>
struct layer {
    // Additional character for auto-inserted null-termination character
    char data[H][W + 1];
    friend std::istream &operator>>(std::istream &in, layer<H, W> &l) {
        for (int i = 0; i < H; ++i) {
            in.get(l.data[i], W + 1);
        }
        return in;
    }
};

int AoC::A::day08() {
    constexpr int height = 6;
    constexpr int width = 25;
    auto layers = in::readFile<in::value<layer<height, width>, 0>>("input/08.txt");

    std::unordered_map<int, int> numZeros;
    for (int i = 0; i < layers.size(); ++i) {
        int zeros = 0;
        auto data = layers[i].data;
        for (int h = 0; h < height; ++h) {
            for(int w = 0; w < width; ++w) {
                if (data[h][w] == '0') {
                    ++zeros;
                }
            }
        }
        numZeros.insert({ i, zeros });
    }

    auto leastZeros = std::min_element(numZeros.begin(), numZeros.end(), [](auto a, auto b) {
        return a.second < b.second;
    });

    auto data = layers[leastZeros->first].data;
    int ones = 0;
    int twos = 0;
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            switch (data[h][w]) {
                case '2': ++twos; break;
                case '1': ++ones; break;
            }
        }
    }

    return ones * twos;
}

int AoC::B::day08() {
    constexpr int height = 6;
    constexpr int width = 25;
    auto layers = in::readFile<in::value<layer<height, width>, 0>>("input/08.txt");

    const auto numLayers = layers.size();
    char image[height][width];
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            for (int i = 0; i < numLayers; ++i) {
                auto pixel = layers[i].data[h][w];
                if (layers[i].data[h][w] != Transparent) {
                    image[h][w] = pixel;
                    break;
                }
            }
        }
    }

    auto output = std::ofstream { "output/08.txt" };
    if (!output) {
        console::fatal("Failed to open output file.");
    }
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            output << (image[h][w] == White ? '1' : ' ');
        }
        output << std::endl;
    }
    return 0;
}