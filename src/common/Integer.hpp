#pragma once
#include <limits>
#include <type_traits>

namespace Integer {
    using Int = long long;
    using uInt = unsigned long long;

    template <typename... Ints>
    using all_integral = std::conjunction<std::is_integral<Ints>...>;

    template <typename Int>
    using enable_if_integral = typename std::enable_if<std::is_integral<Int>::value>::type;

    template <typename... Ints>
    using enable_if_all_integral = typename std::enable_if<all_integral<Ints...>::value>::type;

    template <typename Int>
    using is_unsigned_int = std::conjunction<std::is_integral<Int>, std::is_unsigned<Int>>;

    template <typename Int>
    using is_signed_int = std::conjunction<std::is_integral<Int>, std::is_signed<Int>>;

    template <typename Int>
    using enable_if_unsigned_int = typename std::enable_if<is_unsigned_int<Int>::value>::type;

    template <typename Int>
    using enable_if_signed_int = typename std::enable_if<is_signed_int<Int>::value>::type;

    template <typename A, typename B>
    using both_signed = std::conjunction<std::is_signed<A>, std::is_signed<B>>;

    template <typename A, typename B>
    using both_unsigned = std::conjunction<std::is_unsigned<A>, std::is_unsigned<B>>;

    template <typename A, typename B>
    using first_signed = std::conjunction<std::is_signed<A>, std::is_unsigned<B>>;

    template <typename A, typename B>
    using second_signed = std::conjunction<std::is_unsigned<A>, std::is_signed<B>>;

    template <typename Int, typename Enable = void>
    struct bit_length;

    template <typename Int>
    struct bit_length<Int, enable_if_integral<Int>> {
        static constexpr auto value = std::numeric_limits<Int>::digits;
    };

    template <typename A, typename B, typename... Ints>
    struct bigger_int {
        using type = typename std::conditional<(bit_length<A>::value > bit_length<typename bigger_int<B, Ints...>::type>::value),
            A, typename bigger_int<B, Ints...>::type>::type;
    };

    template <typename A, typename B>
    struct bigger_int<A,B> {
        using type = typename std::conditional<(bit_length<A>::value > bit_length<B>::value), A, B>::type;
    };

    template <typename... Ints>
    using bigger_int_t = typename bigger_int<Ints...>::type;

    template <typename Int, typename Enable = void>
    struct suInt;

    template <typename Int>
    struct suInt<Int, enable_if_unsigned_int<Int>> {
        Int value;
        bool sign;
    };

    template <typename A, typename B>
    static typename std::enable_if<all_integral<A, B>::value, std::make_unsigned_t<bigger_int_t<A, B>>>::type
    modularInverse(A a, B n) {
        // This algorithm relies on negative numbers
        using Int = std::make_signed_t<bigger_int_t<A, B>>;
        Int t = 0, newT = 1;
        Int r = n, newR = a;

        while (newR != 0) {
            auto q = r / newR;

            auto temp = newT;
            newT = t - q * newT;
            t = temp;

            temp = newR;
            newR = r - q * newR;
            r = temp;
        }

        if (r > 1) {
            throw std::runtime_error("a is not invertible");
        }
        if (t < 0) {
            t += n;
        }
        return static_cast<std::make_unsigned_t<Int>>(t);
    }

    template <typename A, typename B>
    inline typename std::enable_if<std::conjunction<all_integral<A, B>, both_unsigned<A, B>>::value, bigger_int_t<A, B>>::type
    mod(A a, B b) {
        return a % b;
    }

    template <typename A, typename B>
    inline typename std::enable_if<std::conjunction<all_integral<A, B>, both_signed<A, B>>::value, bigger_int_t<A, B>>::type
    mod(A a, B b) {
        return ((a % b) + b) % b;
    }

    template <typename A, typename B>
    inline typename std::enable_if<std::conjunction<all_integral<A, B>, first_signed<A, B>>::value, std::make_unsigned_t<bigger_int_t<A, B>>>::type
    mod(A a, B b) {
        if (a < 0) {
            if (b > static_cast<B>(std::numeric_limits<std::make_signed_t<B>>::max())) {
                throw std::overflow_error("Unsigned value out of range for negative modulus.");
            }
            return b - (-a % b);
        }
        return a % b;
    }

    template <typename A, typename B>
    inline typename std::enable_if<std::conjunction<all_integral<A, B>, second_signed<A, B>>::value, std::make_signed_t<bigger_int_t<A, B>>>::type
    mod(A a, B b) {
        if (a > static_cast<A>(std::numeric_limits<std::make_signed_t<A>>::max())) {
            throw std::overflow_error("Unsigned value out of range for negative modulus.");
        }
        return -mod(-static_cast<std::make_signed_t<A>>(a), -b);
    }

    template <typename A, typename B>
    typename std::enable_if<all_integral<A, B>::value, bigger_int_t<A, B>>::type
    gcd(A a, B b) {
        a = std::abs(a);
        b = std::abs(b);
        while (b) {
            Int t = b;
            b = mod(a, b);
            a = t;
        }
        return static_cast<bigger_int_t<A, B>>(a);
    }

    template <typename A, typename B, typename... Ints>
    typename std::enable_if<all_integral<A, B, Ints...>::value, bigger_int_t<A, B, Ints...>>::type
    gcd(A a, B b, Ints... ints) {
        return gcd(a, gcd(b, ints...));
    }

    template <typename A, typename B>
    typename std::enable_if<all_integral<A, B>::value, bigger_int_t<A, B>>::type
    lcm(A a, B b) {
        return std::abs(a * b) / gcd(a, b);
    }

    template <typename A, typename B, typename... Ints>
    typename std::enable_if<all_integral<A, B, Ints...>::value, bigger_int_t<A, B, Ints...>>::type
    lcm(A a, B b, Ints... ints) {
        return lcm(a, lcm(b, ints...));
    }

    template <typename Int>
    inline typename std::enable_if<is_signed_int<Int>::value, Int>::type
    negate(Int a) {
        return -a;
    }

    template <typename Int>
    inline typename std::enable_if<is_unsigned_int<Int>::value, Int>::type
    negate(Int a) {
        return ~a + 1;
    }

    template <typename Int>
    inline typename std::enable_if<std::is_integral<Int>::value, std::make_unsigned_t<Int>>::type
    unsign(Int a) {
        return static_cast<std::make_unsigned_t<Int>>(a);
    }

    template <typename A, typename B>
    inline typename std::enable_if<all_integral<A, B>::value, std::make_unsigned_t<bigger_int_t<A, B>>>::type
    unsignedRightShift(A a, B b) {
        return static_cast<std::make_unsigned_t<A>>(a) >> b;
    }

    template <typename Int>
    typename std::enable_if<std::is_integral<Int>::value && sizeof(Int) * 8 == 32, std::uint32_t>::type
    bitCount(Int a) {
        auto i = Integer::unsign(a);
        i = i - ((i >> 1) & 0x55555555);
        i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
        i = (i + (i >> 4)) & 0x0f0f0f0f;
        i = i + (i >> 8);
        i = i + (i >> 16);
        return static_cast<std::uint32_t>(i & 0x3f);
    }

    template <typename Int>
    typename std::enable_if<std::is_integral<Int>::value && sizeof(Int) * 8 == 64, std::uint32_t>::type
    bitCount(Int a) {
        auto i = Integer::unsign(a);
        i = i - ((i >> 1) & 0x5555555555555555LL);
        i = (i & 0x3333333333333333L) + ((i >> 2) & 0x3333333333333333L);
        i = (i + (i >> 4) & 0x0f0f0f0f0f0f0f0fLL);
        i = i + (i >> 8);
        i = i + (i >> 16);
        i = i + (i >> 32);
        return static_cast<std::uint32_t>(i) & 0x7f;
    }

    template <typename Int>
    typename std::enable_if<std::is_integral<Int>::value && sizeof(Int) * 8 == 32, std::uint32_t>::type
    numberOfLeadingZeros(Int a) {
        if (a == 0) {
            return 32;
        }
        auto i = Integer::unsign(a);
        std::uint32_t n = 1;
        if (i >> 16 == 0) { n += 16; i <<= 16; }
        if (i >> 24 == 0) { n += 8; i <<= 8; }
        if (i >> 28 == 0) { n += 4; i <<= 4; }
        if (i >> 30 == 0) { n += 2; i <<= 2; }
        n -= i >> 31;
        return n;
    }

    template <typename Int>
    typename std::enable_if<std::is_integral<Int>::value && sizeof(Int) * 8 == 64, std::uint32_t>::type
    numberOfLeadingZeros(Int a) {
        if (a == 0) {
            return 64;
        }
        auto i = Integer::unsign(a);
        std::uint32_t n = 1;
        auto x = static_cast<std::uint32_t>(i >> 32);
        if (x == 0) { n += 32; x = static_cast<std::uint32_t>(i); }
        if (x >> 16 == 0) { n += 16; x <<= 16; }
        if (x >> 24 == 0) { n += 8; x <<= 8; }
        if (x >> 28 == 0) { n += 4; x <<= 4; }
        if (x >> 30 == 0) { n += 2; x <<= 2; }
        n -= x >> 31;
        return n;
    }

    template <typename Int>
    typename std::enable_if<std::is_integral<Int>::value && sizeof(Int) * 8 == 32, std::uint32_t>::type
    numberOfTrailingZeros(Int i) {
        std::uint32_t y;
        if (i == 0) {
            return 32;
        }
        std::uint32_t n = 31;
        y = i << 16; if (y != 0) { n -= 16; i = y; }
        y = i << 8; if (y != 0) { n -= 8; i = y;}
        y = i << 4; if (y != 0) { n -= 4; i = y; }
        y = i << 2; if (y != 0) { n -= 2; i = y; }
        return n - ((i << 1) >> 31);
    }

    template <typename Int>
    typename std::enable_if<std::is_integral<Int>::value && sizeof(Int) * 8 == 64, std::uint32_t>::type
    numberOfTrailingZeros(Int i) {
        std::uint32_t x, y;
        if (i == 0) {
            return 64;
        }
        std::uint32_t n = 63;
        y = static_cast<std::uint32_t>(i);
        if (y != 0) { n -= 32; x = y; }
        else x = static_cast<std::uint32_t>(i >> 32);
        y = x << 16; if (y != 0) { n -= 16; x = y; }
        y = x << 8; if (y != 0) { n -= 8; x = y; }
        y = x << 4; if (y != 0) { n -= 4; x = y; }
        y = x << 2; if (y != 0) { n -= 2; x = y; }
        return n - ((x << 1) >> 31);
    }
}