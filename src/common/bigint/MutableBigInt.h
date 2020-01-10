#pragma once

// This class was adapted from the JDK's MutableBigInteger class.
// It is used for more efficient operations (especially when dividing)
// on bigint objects, but it is not really intended to be used externally.

#include "BigInt.h"

class mutable_bigint {
protected:
    using uLong = bigint::uLong;
    using Long = bigint::Long;
    using uInt = bigint::uInt;
    using Int = bigint::Int;
    using int_data = bigint::int_data;

    int_data value;
    uInt intLen;
    uInt offset = 0;

    static const mutable_bigint ZERO;
    static const mutable_bigint ONE;

    static constexpr uInt KNUTH_POW2_THRESH_LEN = 6;
    static constexpr uInt KNUTH_POW2_THRESH_ZEROS = 3;

    mutable_bigint();
    mutable_bigint(uInt val);
    mutable_bigint(const int_data &val);
    mutable_bigint(const bigint &b);
    mutable_bigint(const mutable_bigint &val);

    void ones(uInt n);
    int_data getMagnitude() const;
    uLong toLong() const;
    void clear();
    void reset();
    Int compare(const mutable_bigint &b) const;
    Int compareShifted(const mutable_bigint &b, uInt ints) const;
    uInt getLowestSetBit() const;
    void normalize();

    void primitiveRightShift(uInt n);
    void primitiveLeftShift(uInt n);

    static uInt divadd(const int_data &a, int_data &result, uInt offset);
    static uInt mulsub(const int_data &a, int_data &q, uInt x, uInt len, uInt offset);
    static uInt divaddLong(uInt dh, uInt dl, int_data &result, uInt offset);
    static uInt mulsubLong(uInt dh, uInt dl, int_data &q, uInt x, uInt offset);

    bigint getLower(uInt n) const;
    void keepLower(uInt n);

    Int difference(mutable_bigint &sub);

    mutable_bigint divide2n1n(const mutable_bigint &b, mutable_bigint &quotient);
    mutable_bigint divide3n2n(const mutable_bigint &b, mutable_bigint &quotient);
    mutable_bigint getBlock(uInt index, uInt numBlocks, uInt blockLength) const;
    static void copyAndShift(const int_data &src, uInt srcFrom, uInt srcLen, int_data &dest, uInt destFrom, uInt shift);
    mutable_bigint divideMagnitude(const mutable_bigint &div, mutable_bigint &quotient) const;
    mutable_bigint divideLongMagnitude(uLong ldivisor, mutable_bigint &quotient) const;

    mutable_bigint modInverse(const mutable_bigint &mod) const;

public:
    bigint toBigint(Int sign) const;
    bigint toBigint();

    bool isOne() const;
    bool isZero() const;
    bool isEven() const;
    bool isOdd() const;
    bool isNormal() const;

    void add(const mutable_bigint &addend);
    void addShifted(const mutable_bigint &addend, uInt n);
    void addDisjoint(const mutable_bigint &addend, uInt n);
    void addLower(const mutable_bigint &addend, uInt n);

    Int subtract(const mutable_bigint &sub);
    
    void multiply(const mutable_bigint &y, mutable_bigint &z) const;
    void mul(uInt y, mutable_bigint &z) const;

    uInt divideOneWord(uInt divisor, mutable_bigint &quotient) const;
    mutable_bigint divide(const mutable_bigint &b, mutable_bigint &quotient);
    mutable_bigint divideKnuth(const mutable_bigint &b, mutable_bigint &quotient);
    mutable_bigint divideAndRemainderBurnikelZiegler(const mutable_bigint &b, mutable_bigint &quotient);
    uLong bitLength() const;
    uLong divide(uLong v, mutable_bigint &quotient) const;
    static uLong divWord(uLong, uInt d);

    void safeRightShift(uInt n);
    void rightShift(uInt n);
    void safeLeftShift(uInt n);
    void leftShift(uInt n);

    mutable_bigint mutableModInverse(const mutable_bigint &p) const;
    mutable_bigint modInverseMP2(uInt k) const;
    static Int inverseMod32(uInt val);
    static mutable_bigint modInverseBP2(const mutable_bigint &mod, uInt k);
    static mutable_bigint fixup(mutable_bigint &c, mutable_bigint &p, uInt k);
    mutable_bigint euclidModInverse(uInt k) const;

    friend class bigint;
};