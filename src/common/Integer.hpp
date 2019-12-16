#pragma once
#include <type_traits>

namespace Integer {
    using Int = long long;
    template <typename... Ints>
    using all_integral = std::conjunction<std::is_integral<Ints>...>;

    static Int gcd(Int a, Int b) {
        a = std::abs(a);
        b = std::abs(b);
        while (b) {
            Int t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    template <typename... Ints>
    typename std::enable_if<all_integral<Ints...>::value, Int>::type
        gcd(Int a, Int b, Ints... ints) {
        return gcd(a, gcd(b, ints...));
    }

    static Int lcm(Int a, Int b) {
        return std::abs(a * b) / gcd(a, b);
    }

    template <typename... Ints>
    typename std::enable_if<all_integral<Ints...>::value, Int>::type
        lcm(Int a, Int b, Ints... ints) {
        return lcm(a, lcm(b, ints...));
    }
}