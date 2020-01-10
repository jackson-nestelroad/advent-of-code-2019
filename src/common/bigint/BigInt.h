#pragma once

// This class was adapted from the JDK's BigInteger class.
// It translates over all of the fundamental arithmetic operations,
// but it does not include all of the higher-level features of the JDK class.

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#include "../Console.hpp"
#include "../Integer.hpp"

class bigint {
private:
    using uLong = std::uint64_t;
    using Long = std::int64_t;
    using uInt = std::uint32_t;
    using Int = std::int32_t;
    using int_data = std::vector<uInt>;

    Int sign;
    int_data mag;

    static const bigint ZERO;
    static const bigint ONE;
    static const bigint TWO;
    static const bigint NEGATIVE_ONE;

    static constexpr auto MIN_RADIX = 2;
    static constexpr auto MAX_RADIX = 36;

    static constexpr uInt MAX_MAG_LENGTH = std::numeric_limits<Int>::max() / (std::numeric_limits<Int>::digits + 1) + 1;
    static constexpr uInt KARATSUBA_THRESHOLD = 80;
    static constexpr uInt TOOM_COOK_THRESHOLD = 240;
    static constexpr uInt KARATSUBA_SQUARE_THRESHOLD = 128;
    static constexpr uInt TOOM_COOK_SQUARE_THRESHOLD = 216;
    static constexpr uInt BURNIKEL_ZIEGLER_THRESHOLD = 80;
    static constexpr uInt BURNIKEL_ZIEGLER_OFFSET = 40;
    static constexpr uInt SCHOENHAGE_BASE_CONVERSION_THRESHOLD = 20;

    static std::vector<uInt> digitsPerLong;
    static std::vector<bigint> longRadix;
    static std::vector<uInt> digitsPerInt;
    static std::vector<uInt> intRadix;

    static struct static_initializer_1 {
        static_initializer_1() {
            digitsPerLong = { 0, 0,
                62, 39, 31, 27, 24, 22, 20, 19, 18, 18, 17, 17, 16, 16, 15, 15, 15, 14,
                14, 14, 14, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12 };

            longRadix = { ZERO, ZERO,
                valueOf(0x4000000000000000LL), valueOf(0x383d9170b85ff80bLL),
                valueOf(0x4000000000000000LL), valueOf(0x6765c793fa10079dLL),
                valueOf(0x41c21cb8e1000000LL), valueOf(0x3642798750226111LL),
                valueOf(0x1000000000000000LL), valueOf(0x12bf307ae81ffd59LL),
                valueOf(0xde0b6b3a7640000LL), valueOf(0x4d28cb56c33fa539LL),
                valueOf(0x1eca170c00000000LL), valueOf(0x780c7372621bd74dLL),
                valueOf(0x1e39a5057d810000LL), valueOf(0x5b27ac993df97701LL),
                valueOf(0x1000000000000000LL), valueOf(0x27b95e997e21d9f1LL),
                valueOf(0x5da0e1e53c5c8000LL), valueOf(0xb16a458ef403f19LL),
                valueOf(0x16bcc41e90000000LL), valueOf(0x2d04b7fdd9c0ef49LL),
                valueOf(0x5658597bcaa24000LL), valueOf(0x6feb266931a75b7LL),
                valueOf(0xc29e98000000000LL), valueOf(0x14adf4b7320334b9LL),
                valueOf(0x226ed36478bfa000LL), valueOf(0x383d9170b85ff80bLL),
                valueOf(0x5a3c23e39c000000LL), valueOf(0x4e900abb53e6b71LL),
                valueOf(0x7600ec618141000LL), valueOf(0xaee5720ee830681LL),
                valueOf(0x1000000000000000LL), valueOf(0x172588ad4f5f0981LL),
                valueOf(0x211e44f7d02c1000LL), valueOf(0x2ee56725f06e5c71LL),
                valueOf(0x41c21cb8e1000000LL) };

            digitsPerInt = { 0, 0, 30, 19, 15, 13, 11,
                11, 10, 9, 9, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6,
                6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5 };

            intRadix = { 0, 0,
                0x40000000, 0x4546b3db, 0x40000000, 0x48c27395, 0x159fd800,
                0x75db9c97, 0x40000000, 0x17179149, 0x3b9aca00, 0xcc6db61,
                0x19a10000, 0x309f1021, 0x57f6c100, 0xa2f1b6f,  0x10000000,
                0x18754571, 0x247dbc80, 0x3547667b, 0x4c4b4000, 0x6b5a6e1d,
                0x6c20a40,  0x8d2d931,  0xb640000,  0xe8d4a51,  0x1269ae40,
                0x17179149, 0x1cb91000, 0x23744899, 0x2b73a840, 0x34e63b41,
                0x40000000, 0x4cfa3cc1, 0x5c13d840, 0x6d91b519, 0x39aa400 };

        }
    } _init1;

    static constexpr Int MAX_CONSTANT = 16;
    static std::vector<bigint> posConst;
    static std::vector<bigint> negConst;
    static std::vector<std::vector<bigint>> powerCache;
    static std::vector<double> logCache;
    static const double LOG_TWO;

    static struct static_initializer_2 {
        static_initializer_2() {
            posConst.resize(MAX_CONSTANT + 1);
            negConst.resize(MAX_CONSTANT + 1);
            for (uInt i = 1; i <= MAX_CONSTANT; ++i) {
                int_data magnitude { i };
                posConst[i] = { magnitude, 1 };
                negConst[i] = { magnitude, -1 };
            }

            // Initialize the cache of radix^(2^x) values used for base conversion
            // with just the very first value. Additional values will be created
            // on demand.
            powerCache.resize(MAX_RADIX + 1);
            logCache.resize(MAX_RADIX + 1);

            for (uInt i = MIN_RADIX; i <= MAX_RADIX; ++i) {
                powerCache[i] = { bigint::valueOf(static_cast<uLong>(i)) };
                logCache[i] = std::log(static_cast<double>(i));
            }
        }
    } _init2;

    bigint(const int_data &mag, Int sign);

    void checkRange() const;
    static void reportOverflow();
    static void destructiveMulAdd(int_data &x, uInt y, uInt z);
    static uInt mulAdd(int_data &out, const int_data &in, uInt offset, uInt len, uInt  k);
    static uInt addOne(int_data &a, uInt offset, uInt mlen, uInt carry);

    Int compareMagnitude(const bigint &val) const;
    static int_data trustedStripLeadingZeroInts(const int_data &val);

    static int_data add(const int_data &x, uLong val);
    static int_data add(const int_data &x, const int_data &y);
    static int_data subtract(uLong val, const int_data &little);
    static int_data subtract(const int_data &big, uLong val);
    static int_data subtract(const int_data &big, const int_data &little);

    static bigint multiplyByInt(const int_data &x, uInt y, Int sign);
    static void multiplyToLen(const int_data &x, uLong xlen, const int_data &y, uLong ylen, int_data &z);
    static bigint multiplyKaratsuba(const bigint &x, const bigint &y);
    static bigint multiplyToomCook3(const bigint &a, const bigint &b);
    bigint getToomSlice(uInt lowerSize, uInt upperSize, uInt slice, uInt fullsize) const;
    bigint exactDivideBy3() const;

    bigint square() const;
    static void squareToLen(const int_data &x, uInt len, int_data &z);
    bigint squareKaratsuba() const;
    bigint squareToomCook3() const;

    bigint getLower(uInt n) const;
    bigint getUpper(uInt n) const;

    static int_data shiftLeft(const int_data &mag, uInt n);
    bigint shiftRightImpl(uInt n) const;
    static void increment(int_data &val);
    static void decrement(int_data &val);
    static void primitiveRightShift(int_data &a, uInt len, uInt n);
    static void primitiveLeftShift(int_data &a, uInt len, uInt n);

    static bigint getRadixConversionCache(uInt radix, uInt exponent);

    uInt intLength() const;
    uInt signBit() const;
    Int signInt() const;
    uInt getInt(Int n) const;
    Int firstNonzeroIntNum() const;
    static uInt bitLengthForInt(uInt x);
    std::string toString(uInt radix) const;
    std::string smallToString(uInt radix) const;
    static void toString(const bigint &u, std::string &str, uInt radix, Int digits);

public:
    bigint();
    bigint(Int val);
    bigint(uInt val);
    bigint(Long val);
    bigint(uLong val);
    bigint(const bigint &val);
    static bigint valueOf(Long val);
    static bigint valueOf(uLong val);

    // Maybe one day...
    // bigint(const std::string &str);

    uInt bitLength() const;
    Int getLowestSetBit() const;

    bigint add(const bigint &val) const;
    bigint subtract(const bigint &val) const;
    bigint multiply(const bigint &val) const;
    bigint divide(const bigint &val) const;
    bigint remainder(const bigint &val) const;
    std::pair<bigint, bigint> divideAndRemainder(const bigint &val) const;

    bigint shiftLeft(uInt n) const;
    bigint shiftRight(uInt n) const;

    bigint abs() const;
    bigint negate() const;

    bigint pow(uInt exponent) const;

    bigint mod(const bigint &val) const;
    bigint modInverse(const bigint &m) const;

    Int compare(const bigint &val) const;

    uLong toULong() const;
    std::string toString() const;

    bigint operator+(const bigint &val) const;
    bigint operator-(const bigint &val) const;
    bigint operator*(const bigint &val) const;
    bigint operator/(const bigint &val) const;
    bigint operator%(const bigint &val) const;
    bigint operator<<(uInt val) const;
    bigint operator>>(uInt val) const;
    bigint operator+() const;
    bigint operator-() const;
    bigint &operator+=(const bigint &val);
    bigint &operator-=(const bigint &val);
    bigint &operator*=(const bigint &val);
    bigint &operator/=(const bigint &val);
    bigint &operator%=(const bigint &val);
    bigint &operator<<=(uInt val);
    bigint &operator>>=(uInt val);

    bool operator<(const bigint &val) const;
    bool operator==(const bigint &val) const;
    bool operator!=(const bigint &val) const;
    bool operator>(const bigint &val) const;
    bool operator<=(const bigint &val) const;
    bool operator>=(const bigint &val) const;

    //bigint &operator++();
    //bigint operator++(int);
    //bigint &operator--();
    //bigint operator--(int);

    operator std::string() const;
    friend std::ostream &operator<<(std::ostream &out, const bigint &val);

    friend class mutable_bigint;
};