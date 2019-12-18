#include "../AdventOfCode2019.hpp"

template <int N>
struct PowerOf2 {
    constexpr static long long value = 1 << N;
};

template <int N, int M>
struct Pattern {
    constexpr static int value = ((M & PowerOf2<N - 1>::value) ? 1 : 0) * ((M & PowerOf2<N>::value) ? -1 : 1);
};

constexpr int pattern[4] { 0, 1, 0, -1 };

inline int coefficient(int i, int n) {
    return pattern[((i + 1) / (n + 1)) % 4];
}

void FFT(std::vector<int> &phase, int iterations) {
    const std::size_t length = phase.size();
    for (int p = 0; p < iterations; ++p) {
        std::vector<int> prev = phase;
        for (int n = 0; n < length; ++n) {
            int sum = 0;
            for (int i = 0; i < length; ++i) {
                sum += prev[i] * coefficient(i, n);
            }
            phase[n] = std::abs(sum % 10);
        }
    }
}

void optimizedFFT(std::vector<int> &phase, int iterations) {
    const std::size_t length = phase.size();
    for (int p = 0; p < iterations; ++p) {
        std::vector<int> prev = phase;
        int step = 4;
        int neg = 2;
        for (int n = 0; n < length; ++n, step += 4, neg += 3) {
            int sum = 0;
            for (int i = 0; i <= n; ++i) {
                for (int j = n; j < length - i; j += step) {
                    sum += prev[i + j];
                }
                for (int j = neg; j < length - i; j += step) {
                    sum -= prev[i + j];
                }
            }
            phase[n] = std::abs(sum) % 10;
        }
    }
}

int AoC::A::day16() {
    auto input = in::readFile<in::value<char, 0>>("input/16.txt");
    std::vector<int> phase;
    std::transform(input.begin(), input.end(), std::back_inserter(phase), [](char a) { return a - '0'; });

    optimizedFFT(phase, 100);

    int result = 0;
    for (int i = 7, mul = 1; i >= 0; --i, mul *= 10) {
        result += phase[i] * mul;
    }
    return result;
}

int AoC::B::day16() {
    auto input = in::readFile<in::value<char, 0>>("input/16.txt");
    std::vector<int> phase;
    std::transform(input.begin(), input.end(), std::back_inserter(phase), [](char a) { return a - '0'; });

    int offset = 0;
    for (int i = 6, mul = 1; i >= 0; --i, mul *= 10) {
        offset += phase[i] * mul;
    }
    const int l = static_cast<int>(phase.size());
    const int L = 10'000 * l;
    const int size = L - offset;

    std::vector<int> data(size);
    for (int i = 0; i < size; ++i) {
        data[i] = phase[(offset + i) % l];
    }

    // This optimization of FFT is only applicable if offset >= (L / 2)
    for (int p = 0; p < 100; ++p) {
        std::vector<int> prev = data;

        int sum = 0;
        for (int n = size - 1; n >= 0; --n) {
            sum += prev[n];
            data[n] = sum % 10;
        }
    }

    int result = 0;
    for (int i = 7, mul = 1; i >= 0; --i, mul *= 10) {
        result += data[i] * mul;
    }
    return result;
}