#include "../AdventOfCode2019.hpp"

template <int N>
struct password {
    int digits[N];

    password(int num) {
        for (int i = 0; i < N; ++i, num /= 10) {
            digits[i] = num % 10;
        }
    }

    // Will increment to next valid password by copying first non-overflow digit to all previous digits
    password<N> &operator++() {
        for (int i = 0; i < N; ++i) {
            ++digits[i];
            if (digits[i] != 10) {
                while (--i >= 0) {
                    digits[i] = digits[i + 1];
                }
                return *this;
            }
        }
        return *this;
    }

    operator int() const {
        int out = 0;
        int mult = 1;
        for (int i = 0; i < N; ++i, mult *= 10) {
            out += digits[i] * mult;
        }
        return out;
    }
};

template <int N>
bool operator<(const password<N> &a, const password<N> &b) {
    return static_cast<int>(a) < static_cast<int>(b);
}

// Change password to the immediately next valid one
template <int N>
void initialize(password<N> &a) {
    for (int i = N - 1; i >= 0; --i) {
        if (a.digits[i] < a.digits[i + 1]) {
            a.digits[i] = a.digits[i + 1];
        }
    }
}

// Only need to validate if at least two adjacent digits are the same
template <int N>
bool atLeastTwoAdjacent(const password<N> &a) {
    for (int i = 0; i < N - 1; ++i) {
        if (a.digits[i] == a.digits[i + 1]) {
            return true;
        }
    }
    return false;
}

template <int N>
bool exactlyTwoAdjacent(const password<N> &a) {
    for (int i = 0; i < N - 1;) {
        int j = i;
        while (j < N && a.digits[i] == a.digits[++j]);
        // Exactly two adjacent digits are the same
        if (j - i == 2) {
            return true;
        }
        // Move to first adjacent integer that did not match
        i = j;
    }
    return false;
}

int AoC::A::day04() {
    auto input = in::readFile<in::value<int, '-'>>("input/04.txt");
    password<6> pass { input[0] };
    password<6> end { input[1] };
    initialize(pass);
    int count = 0;
    for (; pass < end; ++pass) {
        if (atLeastTwoAdjacent(pass)) {
            ++count;
        }
    }
    return count;
}

int AoC::B::day04() {
    auto input = in::readFile<in::value<int, '-'>>("input/04.txt");
    password<6> pass { input[0] };
    password<6> end { input[1] };
    initialize(pass);
    int count = 0;
    for (; pass < end; ++pass) {
        if (exactlyTwoAdjacent(pass)) {
            ++count;
        }
    }
    return count;
}