#include "MutableBigInt.h"
#include "SignedMutableBigInt.h"

const mutable_bigint mutable_bigint::ZERO { 0 };
const mutable_bigint mutable_bigint::ONE { 1 };

mutable_bigint::mutable_bigint() {
    value.resize(1);
    intLen = 0;
}

mutable_bigint::mutable_bigint(uInt val) {
    if (val == 0) {
        intLen = 0;
    }
    else {
        value.push_back(val);
        intLen = 1;
    }
}

mutable_bigint::mutable_bigint(const int_data &val) {
    value = val;
    intLen = static_cast<uInt>(val.size());
}

mutable_bigint::mutable_bigint(const bigint &b) {
    value = b.mag;
    intLen = static_cast<uInt>(b.mag.size());
}

mutable_bigint::mutable_bigint(const mutable_bigint &val) {
    intLen = val.intLen;
    std::copy(val.value.begin() + val.offset, 
        val.value.begin() + val.offset + intLen, 
        std::back_inserter(value));
}

void mutable_bigint::ones(uInt n) {
    if (n > static_cast<uInt>(value.size())) {
        value.resize(n);
    }
    std::fill(value.begin(), value.end(), static_cast<uInt>(-1));
    offset = 0;
    intLen = n;
}

mutable_bigint::int_data mutable_bigint::getMagnitude() const {
    int_data mag(intLen);
    std::copy(value.begin() + offset, value.begin() + offset + intLen, mag.begin());
    return mag;
}

mutable_bigint::uLong mutable_bigint::toLong() const {
    if (intLen > 2) {
        throw std::out_of_range("This mutable bigint exceeds 64 bits.");
    }
    if (intLen == 0) {
        return 0;
    }
    uLong d = value[offset];
    return intLen == 2 ? ((d << 32) | value[offset + 1ULL]) : d;
}

bigint mutable_bigint::toBigint(Int sign) const {
    if (intLen == 0 || sign == 0) {
        return bigint::ZERO;
    }
    return { getMagnitude(), sign };
}

bigint mutable_bigint::toBigint() {
    normalize();
    return toBigint(isZero() ? 0 : 1);
}

void mutable_bigint::clear() {
    offset = intLen = 0;
    std::fill(value.begin(), value.end(), 0);
}

void mutable_bigint::reset() {
    offset = intLen = 0;
}

mutable_bigint::Int mutable_bigint::compare(const mutable_bigint &b) const {
    uInt blen = b.intLen;
    if (intLen < blen) {
        return -1;
    }
    if (intLen > blen) {
        return 1;
    }
    for (uInt i = offset, j = b.offset; i < intLen + offset; ++i, ++j) {
        uInt b1 = value[i];
        uInt b2 = b.value[j];
        if (b1 < b2) {
            return -1;
        }
        if (b1 > b2) {
            return 1;
        }
    }
    return 0;
}

mutable_bigint::Int mutable_bigint::compareShifted(const mutable_bigint &b, uInt ints) const {
    uInt blen = b.intLen;
    uInt alen = intLen - ints;
    if (ints > intLen || alen < blen) {
        return -1;
    }
    if (alen > blen) {
        return 1;
    }
    for (uInt i = offset, j = b.offset; i < alen + offset; ++i, ++j) {
        uInt b1 = value[i];
        uInt b2 = b.value[j];
        if (b1 < b2) {
            return -1;
        }
        if (b1 > b2) {
            return 1;
        }
    }
    return 0;
}

mutable_bigint::uInt mutable_bigint::getLowestSetBit() const {
    if (intLen == 0) {
        return static_cast<uInt>(-1);
    }
    uLong j;
    uInt b;
    for (j = intLen - 1; (j > 0) && (value[j + offset] == 0); --j);
    b = value[j + offset];
    if (b == 0) {
        return -1;
    }
    return static_cast<uInt>(((intLen - 1 - j) << 5) + Integer::numberOfTrailingZeros(b));
}

void mutable_bigint::normalize() {
    if (intLen == 0) {
        offset = 0;
        return;
    }

    uInt index = offset;
    if (value[index] != 0) {
        return;
    }

    uInt indexBound = index + intLen;
    do {
        ++index;
    } while (index < indexBound && value[index] == 0);

    uInt numZeros = index - offset;
    intLen -= numZeros;
    offset = intLen == 0 ? 0 : offset + numZeros;
}

bool mutable_bigint::isOne() const {
    return (intLen == 1) && (value[offset] == 1);
}

bool mutable_bigint::isZero() const {
    return intLen == 0;
}

bool mutable_bigint::isEven() const {
    return (intLen == 0) || ((value[static_cast<uLong>(offset) + intLen - 1] & 1) == 0);
}

bool mutable_bigint::isOdd() const {
    return intLen == 0 ? false : ((value[static_cast<uLong>(offset) + intLen - 1] & 1) == 1);
}

bool mutable_bigint::isNormal() const {
    if (static_cast<uLong>(intLen) + offset > value.size()) {
        return false;
    }
    if (intLen == 0) {
        return true;
    }
    return value[offset] != 0;
}

void mutable_bigint::safeRightShift(uInt n) {
    if (n / 32 >= intLen) {
        reset();
    }
    else {
        rightShift(n);
    }
}

void mutable_bigint::rightShift(uInt n) {
    if (intLen == 0) {
        return;
    }
    uInt nInts = n >> 5;
    uInt nBits = n & 0x1F;
    intLen -= nInts;
    if (nBits == 0) {
        return;
    }
    uInt bitsInHighWord = bigint::bitLengthForInt(value[offset]);
    if (nBits >= bitsInHighWord) {
        primitiveLeftShift(32 - nBits);
        --intLen;
    }
    else {
        primitiveRightShift(nBits);
    }
}

void mutable_bigint::safeLeftShift(uInt n) {
    if (static_cast<Int>(n) > 0) {
        leftShift(n);
    }
}

void mutable_bigint::leftShift(uInt n) {
    if (intLen == 0) {
        return;
    }
    uInt nInts = n >> 5;
    uInt nBits = n & 0x1F;
    uInt bitsInHighWord = bigint::bitLengthForInt(value[offset]);

    // If shift can be done without moving words, do so
    if (n <= (32 - bitsInHighWord)) {
        primitiveLeftShift(nBits);
        return;
    }

    uInt newLen = intLen + nInts + 1;
    if (nBits <= (32 - bitsInHighWord)) {
        --newLen;
    }
    if (value.size() < newLen) {
        // The array must grow
        value.resize(newLen);
        std::copy(value.begin() + offset,
            value.begin() + offset + intLen,
            value.begin());
        std::fill(value.begin() + intLen, value.end(), 0);
        intLen = newLen;
        offset = 0;
    }
    else if (value.size() - offset >= newLen) {
        // Use space on right
        std::fill(value.begin() + offset + intLen,
            value.begin() + offset + newLen, 0);
    }
    else {
        // Use space on left
        std::copy(value.begin() + offset,
            value.begin() + offset + intLen,
            value.begin());
        std::fill(value.begin() + intLen, value.begin() + newLen, 0);
        offset = 0;
    }
    intLen = newLen;
    if (nBits == 0) {
        return;
    }
    if (nBits <= (32 - bitsInHighWord)) {
        primitiveLeftShift(nBits);
    }
    else {
        primitiveRightShift(32 - nBits);
    }
}

void mutable_bigint::primitiveRightShift(uInt n) {
    uInt n2 = 32 - n;
    for (uInt i = offset + intLen - 1, c = value[i]; i > offset; --i) {
        uInt b = c;
        c = value[i - 1];
        value[i] = (c << n2) | (b >> n);
    }
    value[offset] >>= n;
}

void mutable_bigint::primitiveLeftShift(uInt n) {
    uInt n2 = 32 - n;
    const uInt m = offset + intLen - 1;
    for (uInt i = offset, c = value[i]; i < m; ++i) {
        uInt b = c;
        c = value[i + 1ULL];
        value[i] = (b << n) | (c >> n2);
    }
    value[m] <<= n;
}

mutable_bigint::uInt mutable_bigint::divadd(const int_data &a, int_data &result, uInt offset) {
    uLong carry = 0;
    for (Int j = static_cast<Int>(a.size()) - 1; j >= 0; --j) {
        uLong sum = static_cast<uLong>(a[j]) + result[static_cast<uLong>(j) + offset] + carry;
        result[static_cast<uLong>(j) + offset] = static_cast<uInt>(sum);
        carry = sum >> 32;
    }
    return static_cast<uInt>(carry);
}

mutable_bigint::uInt mutable_bigint::mulsub(const int_data &a, int_data &q, uInt x, uInt len, uInt offset) {
    uLong xlong = x;
    uLong carry = 0;
    offset += len;
    for (Int j = static_cast<Int>(len - 1); j >= 0; --j) {
        uLong product = a[j] * xlong + carry;
        uLong difference = q[offset] - product;
        q[offset--] = static_cast<uInt>(difference);
        carry = (product >> 32) + ((static_cast<uInt>(difference) > static_cast<uInt>(~product)) ? 1 : 0);
    }
    return static_cast<uInt>(carry);
}

bigint mutable_bigint::getLower(uInt n) const {
    if (intLen == 0) {
        return bigint::ZERO;
    }
    else if (intLen < n) {
        return toBigint(1);
    }
    else {
        // Strip zeros
        uInt len = n;
        while (len > 0 && value[static_cast<uLong>(offset) + intLen - len] == 0) {
            --len;
        }
        Int sign = len > 0 ? 1 : 0;
        int_data mag(len);
        std::copy(value.begin() + offset + intLen - len, value.begin() + offset + intLen, mag.begin());
        return { mag, sign };
    }
}

void mutable_bigint::keepLower(uInt n) {
    if (intLen >= n) {
        offset += intLen - n;
        intLen = n;
    }
}

void mutable_bigint::add(const mutable_bigint &addend) {
    uLong x = intLen;
    uLong y = addend.intLen;
    uLong resultLen = std::max(x, y);
    if (value.size() < resultLen) {
        value.resize(resultLen);
    }

    uInt rstart = static_cast<uInt>(value.size() - 1);
    uLong sum;
    uLong carry = 0;

    // Add common parts of both numbers
    while (x > 0 && y > 0) {
        --x; --y;
        sum = static_cast<uLong>(value[x + offset]) + addend.value[y + addend.offset] + carry;
        value[rstart--] = static_cast<uInt>(sum);
        carry = sum >> 32;
    }

    // Add remainder of the longer number
    while (x > 0) {
        --x;
        if (carry == 0 && rstart == (x + offset)) {
            return;
        }
        sum = static_cast<uLong>(value[x + offset]) + carry;
        value[rstart--] = static_cast<uInt>(sum);
        carry = sum >> 32;
    }
    while (y > 0) {
        --y;
        sum = static_cast<uLong>(addend.value[y + addend.offset]) + carry;
        value[rstart--] = static_cast<uInt>(sum);
        carry = sum >> 32;
    }

    if (carry > 0) {
        ++resultLen;
        if (value.size() < resultLen) {
            value.insert(value.begin(), 1);
        }
        else {
            value[rstart--] = 1;
        }
    }

    intLen = static_cast<uInt>(resultLen);
    offset = static_cast<uInt>(value.size() - resultLen);
}

void mutable_bigint::addShifted(const mutable_bigint &addend, uInt n) {
    if (addend.isZero()) {
        return;
    }

    uLong x = intLen;
    uLong y = static_cast<uLong>(addend.intLen) + n;
    uLong resultLen = std::max(x, y);
    if (value.size() < resultLen) {
        value.resize(resultLen);
    }

    uInt rstart = static_cast<uInt>(value.size() - 1);
    uLong sum;
    uLong carry = 0;

    // Add common parts of both numbers
    while (x > 0 && y > 0) {
        --x; --y;
        uInt bval = ((y + addend.offset) < addend.value.size()) ? addend.value[y + addend.offset] : 0;
        sum = static_cast<uLong>(value[x + offset]) + bval + carry;
        value[rstart--] = static_cast<uInt>(sum);
        carry = sum >> 32;
    }

    // Add remainder of longer number
    while (x > 0) {
        --x;
        if (carry == 0 && rstart == (x + offset)) {
            return;
        }
        sum = static_cast<uLong>(value[x + offset]) + carry;
        value[rstart--] = static_cast<uInt>(sum);
        carry = sum >> 32;
    }
    while (y > 0) {
        --y;
        uInt bval = ((y + addend.offset) < addend.value.size()) ? addend.value[y + addend.offset] : 0;
        sum = bval + carry;
        value[rstart--] = static_cast<uInt>(sum);
        carry = sum >> 32;
    }
    
    if (carry > 0) {
        ++resultLen;
        if (value.size() < resultLen) {
            value.insert(value.begin(), 1);
        }
        else {
            value[rstart--] = 1;
        }
    }

    intLen = static_cast<uInt>(resultLen);
    offset = static_cast<uInt>(value.size() - resultLen);
}

void mutable_bigint::addDisjoint(const mutable_bigint &addend, uInt n) {
    if (addend.isZero()) {
        return;
    }

    uLong x = static_cast<uLong>(intLen);
    uLong y = static_cast<uLong>(addend.intLen) + n;
    uLong resultLen = std::max(x, y);
    if (value.size() < resultLen) {
        value.resize(resultLen);
    }
    else {
        std::fill(value.begin() + offset + intLen, value.end(), 0);
    }

    uInt rstart = static_cast<uInt>(value.size()) - 1;

    std::copy(value.begin() + offset,
        value.begin() + offset + x,
        value.begin() + rstart + 1 - x);
    y -= x;
    rstart -= static_cast<uInt>(x);

    uLong len = std::min(y, addend.value.size() - addend.offset);
    std::copy(addend.value.begin() + addend.offset, 
        addend.value.begin() + addend.offset + len, 
        value.begin() + rstart + 1 - y);

    // Zero the gap
    std::fill(value.begin() + rstart + 1 - y + len, value.begin() + rstart + 1, 0);

    intLen = static_cast<uInt>(resultLen);
    offset = static_cast<uInt>(value.size() - resultLen);
}

void mutable_bigint::addLower(const mutable_bigint &addend, uInt n) {
    mutable_bigint a(addend);
    if (a.offset + a.intLen >= n) {
        a.offset += a.intLen - n;
        a.intLen = n;
    }
    a.normalize();
    add(a);
}

mutable_bigint::Int mutable_bigint::subtract(const mutable_bigint &sub) {
    const mutable_bigint *first = this;
    const mutable_bigint *second = &sub;

    Int sign = first->compare(*second);

    if (sign == 0) {
        reset();
        return 0;
    }
    if (sign < 0) {
        std::swap(first, second);
    }

    const mutable_bigint &a = *first;
    const mutable_bigint &b = *second;

    uLong resultLen = a.intLen;
    if (value.size() < resultLen) {
        value.resize(resultLen);
    }

    uLong diff = 0;
    uLong x = a.intLen;
    uLong y = b.intLen;
    uInt rstart = static_cast<uInt>(value.size() - 1);

    // Subtract common parts of both numbers
    while (y > 0) {
        --x; --y;
        diff = static_cast<uLong>(a.value[x + a.offset]) - b.value[y + b.offset] 
            - static_cast<uInt>(Integer::negate(diff >> 32));
        value[rstart--] = static_cast<uInt>(diff);
    }
    // Subtract remainder of longer number
    while (x > 0) {
        --x;
        diff = static_cast<uLong>(a.value[x + a.offset]) - static_cast<uInt>(Integer::negate(diff >> 32));
        value[rstart--] = static_cast<uInt>(diff);
    }

    intLen = static_cast<uInt>(resultLen);
    offset = static_cast<uInt>(value.size() - resultLen);
    normalize();
    return sign;
}

mutable_bigint::Int mutable_bigint::difference(mutable_bigint &sub) {
    mutable_bigint *first = this;
    mutable_bigint *second = &sub;

    Int sign = first->compare(*second);

    if (sign == 0) {
        return 0;
    }
    if (sign < 0) {
        std::swap(first, second);
    }

    mutable_bigint &a = *first;
    const mutable_bigint &b = *second;

    uLong diff = 0;
    uLong x = a.intLen;
    uLong y = intLen;

    // Subtract commmon parts of both numbers
    while (y > 0) {
        --x; --y;
        diff = static_cast<uLong>(a.value[a.offset + x]) - b.value[b.offset + y]
            - static_cast<uInt>(Integer::negate(diff >> 32));
        a.value[a.offset + x] = static_cast<uInt>(diff);
    }
    // Subtract remainder of longer number
    while (x > 0) {
        --x;
        diff = static_cast<uLong>(a.value[a.offset + x]) - static_cast<uInt>(Integer::negate(diff >> 32));
        a.value[a.offset + x] = static_cast<uInt>(diff);
    }

    a.normalize();
    return sign;
}

void mutable_bigint::multiply(const mutable_bigint &y, mutable_bigint &z) const {
    uLong xLen = intLen;
    uLong yLen = y.intLen;
    uLong newLen = xLen + yLen;

    if (z.value.size() < newLen) {
        z.value.resize(newLen);
    }
    z.offset = 0;
    z.intLen = static_cast<uInt>(newLen);

    // The first iteration is hoisted out of the loop to avoid extra add
    uLong carry = 0;
    for (Int j = static_cast<Int>(yLen - 1), k = static_cast<Int>(newLen - 1); j >= 0; --j, --k) {
        uLong product = static_cast<uLong>(y.value[static_cast<uLong>(j) + y.offset]) * value[xLen - 1 + offset] + carry;
        z.value[k] = static_cast<uInt>(product);
        carry = product >> 32;
    }
    z.value[xLen - 1] = static_cast<uInt>(carry);

    // Perform the multiplication word by word
    for (Int i = static_cast<uInt>(xLen - 2); i >= 0; --i) {
        carry = 0;
        for (Int j = static_cast<Int>(yLen - 1), k = static_cast<Int>(yLen + i); j >= 0; --j, --k) {
            uLong product = static_cast<uLong>(y.value[static_cast<uLong>(j) + y.offset])
                * value[static_cast<uLong>(i) + offset] + z.value[k] + carry;
            z.value[k] = static_cast<uInt>(product);
            carry = product >> 32;
        }
        z.value[i] = static_cast<uInt>(carry);
    }

    // Remove leading zeros from product
    z.normalize();
}

void mutable_bigint::mul(uInt y, mutable_bigint &z) const {
    if (y == 1) {
        z.value.resize(intLen);
        std::copy(value.begin() + offset, value.begin() + offset + intLen, z.value.begin());
        z.offset = 0;
        return;
    }

    if (y == 0) {
        z.clear();
        return;
    }

    // Perform the multiplication word by word
    uLong ylong = y;
    if (z.value.size() < static_cast<uLong>(intLen) + 1) {
        z.value.resize(static_cast<uLong>(intLen) + 1);
    }

    uLong carry = 0;
    for (Int i = static_cast<uInt>(intLen - 1); i >= 0; --i) {
        uLong product = ylong * value[static_cast<uLong>(i) + offset] + carry;
        z.value[static_cast<uLong>(i) + 1] = static_cast<uInt>(product);
        carry = product >> 32;
    }

    if (carry == 0) {
        z.offset = 1;
        z.intLen = intLen;
    }
    else {
        z.offset = 0;
        z.intLen = intLen + 1;
        z.value[0] = static_cast<uInt>(carry);
    }
}

mutable_bigint::uInt mutable_bigint::divideOneWord(uInt divisor, mutable_bigint &quotient) const {
    uLong divisorLong = divisor;

    // Special case of one word dividend
    if (intLen == 1) {
        uLong dividendValue = value[offset];
        uInt q = static_cast<uInt>(dividendValue / divisorLong);
        uInt r = static_cast<uInt>(dividendValue - q * divisorLong);
        quotient.value[0] = q;
        quotient.intLen = q == 0 ? 0 : 1;
        quotient.offset = 0;
        return r;
    }

    if (quotient.value.size() < intLen) {
        quotient.value.resize(intLen);
    }
    quotient.offset = 0;
    quotient.intLen = intLen;

    // Normalize the divisor
    uInt shift = Integer::numberOfLeadingZeros(divisor);

    uInt rem = value[offset];
    uLong remLong = rem;
    if (remLong < divisorLong) {
        quotient.value[0] = 0;
    }
    else {
        quotient.value[0] = static_cast<uInt>(remLong / divisorLong);
        rem = static_cast<uInt>(remLong - (quotient.value[0] * divisorLong));
        remLong = rem;
    }
    uInt xlen = intLen;
    while (--xlen > 0) {
        uLong dividendEstimate = (remLong << 32) | value[static_cast<uLong>(offset) + intLen - xlen];
        uInt q;
        if (dividendEstimate >= 0) {
            q = static_cast<uInt>(dividendEstimate / divisorLong);
            rem = static_cast<uInt>(dividendEstimate - q * divisorLong);
        }
        else {
            uLong tmp = divWord(dividendEstimate, divisor);
            q = static_cast<uInt>(tmp);
            rem = static_cast<uInt>(tmp >> 32);
        }
        quotient.value[intLen - xlen] = q;
        remLong = rem;
    }

    quotient.normalize();
    // Unnormalize
    if (shift > 0) {
        return rem % divisor;
    }
    else {
        return rem;
    }
}
mutable_bigint mutable_bigint::divide(const mutable_bigint &b, mutable_bigint &quotient) {
    if (b.intLen < bigint::BURNIKEL_ZIEGLER_THRESHOLD || intLen - b.intLen < bigint::BURNIKEL_ZIEGLER_OFFSET) {
        return divideKnuth(b, quotient);
    }
    else {
        return divideAndRemainderBurnikelZiegler(b, quotient);
    }
}

mutable_bigint mutable_bigint::divideKnuth(const mutable_bigint &b, mutable_bigint &quotient) {
    if (b.intLen == 0) {
        throw std::runtime_error("bigint cannot divide by zero");
    }

    // Dividend is zero
    if (intLen == 0) {
        quotient.intLen = quotient.offset = 0;
        return ZERO;
    }

    Int cmp = compare(b);
    // Dividend less than divisor
    if (cmp < 0) {
        quotient.intLen = quotient.offset = 0;
        return *this;
    }
    // Dividend equal to divisor
    if (cmp == 0) {
        quotient.value[0] = quotient.intLen = 1;
        quotient.offset = 0;
        return ZERO;
    }

    quotient.clear();
    // Special case one word divisor
    if (b.intLen == 1) {
        uInt r = divideOneWord(b.value[b.offset], quotient);
        if (r == 0) {
            return ZERO;
        }
        else {
            return { r };
        }
    }

    // Cancel common powers of two if we're above the KNUTH_POW2_* thresholds
    if (intLen >= KNUTH_POW2_THRESH_LEN) {
        uInt trailingZeroBits = std::min(getLowestSetBit(), b.getLowestSetBit());
        if (trailingZeroBits >= KNUTH_POW2_THRESH_ZEROS * 32) {
            mutable_bigint a = *this;
            mutable_bigint bb = b;
            a.rightShift(trailingZeroBits);
            bb.rightShift(trailingZeroBits);
            mutable_bigint r = a.divideKnuth(bb, quotient);
            r.leftShift(trailingZeroBits);
            return r;
        }
    }

    return divideMagnitude(b, quotient);
}

mutable_bigint mutable_bigint::divideAndRemainderBurnikelZiegler(const mutable_bigint &b, mutable_bigint &quotient) {
    uInt r = intLen;
    uInt s = b.intLen;

    // Clear the quotient
    quotient.offset = quotient.intLen = 0;

    if (r < s) {
        return *this;
    }
    else {
        // Unlike Knuth division, we don't check for common powers of two here because
        // BZ already runs faster if both numbers contain powers of two and canceling the
        // has no additional benefit.
        // Step 1: let m = min(2^k | (2^k) * BURNIKEL_ZIEGLER_THRESHOLD > s }
        uInt m = 1 << (32 - Integer::numberOfLeadingZeros(s / bigint::BURNIKEL_ZIEGLER_THRESHOLD));

        // Step 2a: j = ceil(s / m)
        uInt j = (s + m - 1) / m;
        // Step 2b: block length in 32-bit units
        uInt n = j * m;
        // Block length in bits
        uLong n32 = 32ULL * n;
        // Step 3: sigma = max(T | (2^T)*B < beta^n)
        uInt sigma = static_cast<uInt>(std::max(0LL, static_cast<Long>(n32 - b.bitLength())));
        // Step 4a: shift b so its length is a multiple of n
        mutable_bigint bShifted = b;
        bShifted.safeLeftShift(sigma);
        // Step 4b: shift this by the same amount
        mutable_bigint aShifted = *this;
        aShifted.safeLeftShift(sigma);

        // Step 5: t is the number of blocks needed to accommodate this plus one additional bit
        uInt t = static_cast<uInt>((aShifted.bitLength() + n32) / n32);
        if (t < 2) {
            t = 2;
        }

        // Step 6: conceptually split this into blocks a[t-1], ..., a[0]
        mutable_bigint a1 = aShifted.getBlock(t - 1, t, n);

        // Step 7: z[t-2] = [a[t-1], a[t-2]]
        mutable_bigint z = aShifted.getBlock(t - 2, t, n);
        z.addDisjoint(a1, n);

        // Do schoolbook division on blocks, dividing 2-block numbers by 1-block numbers
        mutable_bigint qi;
        mutable_bigint ri;
        for (uInt i = t - 2; i > 0; --i) {
            // Step 8a: compute (qi, ri) such that z = b * qi + ri
            ri = z.divide2n1n(bShifted, qi);

            // Step 8b: z = [ri, a[i-1]]
            z = aShifted.getBlock(i - 1, t, n);
            z.addDisjoint(ri, n);
            quotient.addShifted(qi, i * n);
        }
        // Final iteration of step 8: do the loop one more time for i = 0 but leave z unchanged
        ri = z.divide2n1n(bShifted, qi);
        quotient.add(qi);

        // Step 9: this and b were shifted, so shift back
        ri.rightShift(sigma);
        return ri;
    }
}

mutable_bigint mutable_bigint::divide2n1n(const mutable_bigint &b, mutable_bigint &quotient) {
    uInt n = b.intLen;
    // Step 1: base case
    if (n % 2 != 0 || n < bigint::BURNIKEL_ZIEGLER_THRESHOLD) {
        return divideKnuth(b, quotient);
    }

    // Step 2: view this as [a1, a2, a3, a4] where each ai is n/2 ints or less
    mutable_bigint aUpper = *this;
    aUpper.safeRightShift(32 * (n / 2));
    keepLower(n / 2);

    // Step 3: q1 = aUpper / b, r1 = aUpper % b
    mutable_bigint q1;
    mutable_bigint r1 = aUpper.divide3n2n(b, q1);

    // Step 4: quotient = [r1, this] / b, r2 = [r1, this] % b
    addDisjoint(r1, n / 2);
    mutable_bigint r2 = divide3n2n(b, quotient);

    // Step 5: let quotient=[q1, quotient] and return r2
    quotient.addDisjoint(q1, n / 2);
    return r2;
}

mutable_bigint mutable_bigint::divide3n2n(const mutable_bigint &b, mutable_bigint &quotient) {
    uInt n = b.intLen / 2;

    // Step 1: view this as [a1,a2,a3] where each ai is n ints or less; let a12 = [a1,a2]
    mutable_bigint a12 = *this;
    a12.safeRightShift(32 * n);

    // Step 2: view b as [b1,b2] where each bi is n ints or less
    mutable_bigint b1 = b;
    b1.safeRightShift(32 * n);
    bigint b2 = b.getLower(n);

    mutable_bigint r, d;
    if (compareShifted(b, n) < 0) {
        // Step 3a: if a1 < b1, let quotient = a12 / b1 and r = 12 % b1
        r = a12.divide2n1n(b1, quotient);

        // Stpe 4: d = quotient * b2
        d = quotient.toBigint().multiply(b2);
    }
    else {
        // Step 3b: if a1 >= b1, let quotient = beta^(n - 1) and r=a12 - b1 * 2^n + b1
        quotient.ones(n);
        a12.add(b1);
        b1.leftShift(32 * n);
        a12.subtract(b1);
        r = a12;

        // Step 4: d = quotient * b2 = (b2 << 32 * n) - b2
        d = b2;
        d.leftShift(32 * n);
        d.subtract(b2);
    }

    // Step 5: r = r * beta^n + a3 - d
    // However, don't subtract d until after while loop so r doesn't become negative
    r.leftShift(32 * n);
    r.addLower(*this, n);

    // Step 6: add b until r >= d
    while (r.compare(d) < 0) {
        r.add(b);
        quotient.subtract(ONE);
    }
    r.subtract(d);

    return r;
}

mutable_bigint mutable_bigint::getBlock(uInt index, uInt numBlocks, uInt blockLength) const {
    uInt blockStart = index * blockLength;
    if (blockStart >= intLen) {
        return { };
    }

    uInt blockEnd = (index == numBlocks - 1) ? intLen : (index + 1) * blockLength;
    if (blockEnd > intLen) {
        return { };
    }

    int_data newVal;
    std::copy(value.begin() + offset + intLen - blockEnd,
        value.begin() + offset + intLen - blockStart,
        std::back_inserter(newVal));
    return { newVal };
}

mutable_bigint::uLong mutable_bigint::bitLength() const {
    if (intLen == 0) {
        return 0;
    }
    return intLen * 32ULL - Integer::numberOfLeadingZeros(value[offset]);
}

mutable_bigint::uLong mutable_bigint::divide(uLong v, mutable_bigint &quotient) const {
    if (v == 0) {
        throw std::runtime_error("bigint cannot divide by zero");
    }

    // Dividend is zero
    if (intLen == 0) {
        quotient.intLen = quotient.offset = 0;
        return 0;
    }

    uInt d = static_cast<uInt>(v >> 32);
    quotient.clear();
    // Special case on word divisor
    if (d == 0) {
        return divideOneWord(static_cast<uInt>(v), quotient);
    }
    else {
        return divideLongMagnitude(v, quotient).toLong();
    }
}

void mutable_bigint::copyAndShift(const int_data &src, uInt srcFrom, uInt srcLen, int_data &dest, uInt destFrom, uInt shift) {
    uInt n2 = 32 - shift;
    uInt c = src[srcFrom];
    for (uLong i = 0; i < srcLen - 1; ++i) {
        uInt b = c;
        c = src[++srcFrom];
        dest[destFrom + i] = (b << shift) | (c >> n2);
    }
    dest[static_cast<uLong>(destFrom) + srcLen - 1] = c << shift;
}

mutable_bigint mutable_bigint::divideMagnitude(const mutable_bigint &div, mutable_bigint &quotient) const {
    // D1: Normalize the divisor
    uInt shift = Integer::numberOfLeadingZeros(div.value[div.offset]);
    // Copy divisor value to protect divisor
    const uInt dlen = div.intLen;
    int_data divisor;
    // Remainder starts as dividend with space for a leading zero
    mutable_bigint rem;
    if (shift > 0) {
        divisor.resize(dlen);
        copyAndShift(div.value, div.offset, dlen, divisor, 0, shift);
        if (Integer::numberOfLeadingZeros(value[offset]) >= shift) {
            rem.value.resize(intLen + 1ULL);
            rem.intLen = intLen;
            rem.offset = 1;
            copyAndShift(value, offset, intLen, rem.value, 1, shift);
        }
        else {
            rem.value.resize(intLen + 2ULL);
            rem.intLen = intLen + 1;
            rem.offset = 1;
            uInt rFrom = offset;
            uInt c = 0;
            uInt n2 = 32 - shift;
            for (uInt i = 1; i < intLen + 1; ++i, ++rFrom) {
                uInt b = c;
                c = value[rFrom];
                rem.value[i] = (b << shift) | (c >> n2);
            }
            rem.value[intLen + 1ULL] = c << shift;
        }
    }
    else {
        std::copy(div.value.begin() + div.offset,
            div.value.begin() + div.offset + div.intLen,
            std::back_inserter(divisor));
        rem.value.resize(intLen + 1ULL);
        std::copy(value.begin() + offset, value.begin() + offset + intLen, rem.value.begin() + 1);
        rem.intLen = intLen;
        rem.offset = 1;
    }

    uInt nlen = rem.intLen;

    // Set the quotient size
    const uInt limit = nlen - dlen + 1;
    if (quotient.value.size() < limit) {
        quotient.value.resize(limit);
        quotient.offset = 0;
    }
    quotient.intLen = limit;
    int_data &q = quotient.value;

    // Must insert leading 0 in rem if its length did not change
    if (rem.intLen == nlen) {
        rem.offset = 0;
        rem.value[0] = 0;
        ++rem.intLen;
    }

    uInt dh = divisor[0];
    uLong dhLong = dh;
    uInt dl = divisor[1];

    // D2: Initialize j
    for (uLong j = 0; j < limit - 1; ++j) {
        // D3: Calculate qhat
        // Estimate qhat
        uInt qhat = 0;
        uInt qrem = 0;
        bool skipCorrection = false;
        uInt nh = rem.value[j + rem.offset];
        uInt nh2 = nh + 0x80000000;
        uInt nm = rem.value[j + 1 + rem.offset];

        if (nh == dh) {
            qhat = ~0;
            qrem = nh + nm;
            skipCorrection = qrem + 0x80000000 < nh2;
        }
        else {
            uLong nChunk = (static_cast<uLong>(nh) << 32) | nm;
            if (nChunk >= 0) {
                qhat = static_cast<uInt>(nChunk / dhLong);
                qrem = static_cast<uInt>(nChunk - (qhat * dhLong));
            }
            else {
                uLong tmp = divWord(nChunk, dh);
                qhat = static_cast<uInt>(tmp);
                qrem = static_cast<uInt>(tmp >> 32);
            }
        }

        if (qhat == 0) {
            continue;
        }

        // Correct qhat
        if (!skipCorrection) {
            uLong nl = rem.value[j + 2 + rem.offset];
            uLong rs = (static_cast<uLong>(qrem) << 32) | static_cast<uInt>(nl);
            uLong estProduct = static_cast<uLong>(dl) * qhat;

            if (estProduct > rs) {
                --qhat;
                qrem = static_cast<uInt>(qrem + dhLong);
                if (qrem >= dhLong) {
                    estProduct -= dl;
                    rs = (static_cast<uLong>(qrem) << 32) | static_cast<uInt>(nl);
                    if (estProduct > rs) {
                        --qhat;
                    }
                }
            }
        }

        // D4: Multiply and subtract
        rem.value[j + rem.offset] = 0;
        uInt borrow = mulsub(divisor, rem.value, qhat, dlen, static_cast<uInt>(j + rem.offset));

        // D5: Test remainder
        if (borrow + 0x80000000 > nh2) {
            // D6: Add back
            divadd(divisor, rem.value, static_cast<uInt>(j + 1 + rem.offset));
            --qhat;
        }

        // Store the quotientdigit
        q[j] = qhat;
    } // D7: Loop on j
    // D3: Caculate qhat
    // Estimate qhat
    uInt qhat = 0;
    uInt qrem = 0;
    bool skipCorrection = false;
    uInt nh = rem.value[limit - 1ULL + rem.offset];
    uInt nh2 = nh + 0x80000000;
    uInt nm = rem.value[static_cast<uLong>(limit) + rem.offset];

    if (nh == dh) {
        qhat = ~0;
        qrem = nh + nm;
        skipCorrection = qrem + 0x80000000 < nh2;
    }
    else {
        uLong nChunk = (static_cast<uLong>(nh) << 32) | nm;
        if (nChunk >= 0) {
            qhat = static_cast<uInt>(nChunk / dhLong);
            qrem = static_cast<uInt>(nChunk - (qhat * dhLong));
        }
        else {
            uLong tmp = divWord(nChunk, dh);
            qhat = static_cast<uInt>(tmp);
            qrem = static_cast<uInt>(tmp >> 32);
        }
    }
    if (qhat != 0) {
        // Correct qhat
        if (!skipCorrection) {
            uLong nl = rem.value[limit + 1ULL + rem.offset];
            uLong rs = (static_cast<uLong>(qrem) << 32) | static_cast<uInt>(nl);
            uLong estProduct = static_cast<uLong>(dl) * qhat;

            if (estProduct > rs) {
                --qhat;
                qrem = static_cast<uInt>(qrem + dhLong);
                if (qrem >= dhLong) {
                    estProduct -= dl;
                    rs = (static_cast<uLong>(qrem) << 32) | static_cast<uInt>(nl);
                    if (estProduct > rs) {
                        --qhat;
                    }
                }
            }
        }

        // D4: Multiply and subtract
        uInt borrow;
        rem.value[limit - 1ULL + rem.offset] = 0;
        borrow = mulsub(divisor, rem.value, qhat, dlen, limit - 1 + rem.offset);

        // D5: Test remainder
        if (borrow + 0x80000000 > nh2) {
            // D6: Add back
            divadd(divisor, rem.value, limit - 1 + 1 + rem.offset);
            --qhat;
        }

        // Store the quotient digit
        q[limit - 1] = qhat;
    }

    // D8: Unnormalize
    if (shift > 0) {
        rem.rightShift(shift);
    }
    rem.normalize();
    quotient.normalize();
    return rem;
}

mutable_bigint mutable_bigint::divideLongMagnitude(uLong ldivisor, mutable_bigint &quotient) const {
    // Remainder starts as dividend with space for a leading zero
    mutable_bigint rem;
    rem.value.resize(intLen + 1ULL);
    std::copy(value.begin() + offset, value.begin() + offset + intLen, rem.value.begin() + 1);
    rem.intLen = intLen;
    rem.offset = 1;

    uInt nlen = rem.intLen;

    uInt limit = nlen - 2 + 1;
    if (quotient.value.size() < limit) {
        quotient.value.resize(limit);
        quotient.offset = 0;
    }
    quotient.intLen = limit;
    int_data &q = quotient.value;

    // D1: Normalize the divisor
    uInt shift = Integer::numberOfLeadingZeros(ldivisor);
    if (shift > 0) {
        ldivisor <<= shift;
        rem.leftShift(shift);
    }

    // Mut insert leading 0 in rem if its length did not change
    if (rem.intLen == nlen) {
        rem.offset = 0;
        rem.value[0] = 0;
        ++rem.intLen;
    }

    uInt dh = static_cast<uInt>(ldivisor >> 32);
    uLong dhLong = dh;
    uInt dl = static_cast<uInt>(ldivisor);

    // D2: Initialize j
    for (uLong j = 0; j < limit; ++j) {
        // D3: Calculate qhat
        // Estimate qhat
        uInt qhat = 0;
        uInt qrem = 0;
        bool skipCorrection = false;
        uInt nh = rem.value[j + rem.offset];
        uInt nh2 = nh + 0x80000000;
        uInt nm = rem.value[j + 1 + rem.offset];

        if (nh == dh) {
            qhat = ~0;
            qrem = nh + nm;
            skipCorrection = qrem + 0x80000000 < nh2;
        }
        else {
            uLong nChunk = (static_cast<uLong>(nh) << 32) | nh2;
            if (nChunk >= 0) {
                qhat = static_cast<uInt>(nChunk / dhLong);
                qrem = static_cast<uInt>(nChunk - (qhat * dhLong));
            }
            else {
                uLong tmp = divWord(nChunk, dh);
                qhat = static_cast<uInt>(tmp);
                qrem = static_cast<uInt>(tmp >> 32);
            }
        }

        if (qhat == 0) {
            continue;
        }

        // Correct qhat
        if (!skipCorrection) {
            uLong nl = rem.value[j + 2 + rem.offset];
            uLong rs = (static_cast<uLong>(qrem) << 32) | static_cast<uInt>(nl);
            uLong estProduct = static_cast<uLong>(dl) * qhat;

            if (estProduct > rs) {
                --qhat;
                qrem = static_cast<uInt>(qrem + dhLong);
                if (qrem >= dhLong) {
                    estProduct -= dl;
                    rs = (static_cast<uLong>(qrem) << 32) | static_cast<uInt>(nl);
                    if (estProduct > rs) {
                        --qhat;
                    }
                }
            }
        }

        // D4: Multiply and subtract
        rem.value[j + rem.offset] = 0;
        uInt borrow = mulsubLong(dh, dl, rem.value, qhat, static_cast<uInt>(j + rem.offset));

        // D5: Test remainder
        if (borrow + 0x80000000 > nh2) {
            // D6: Add back
            divaddLong(dh, dl, rem.value, static_cast<uInt>(j + 1 + rem.offset));
            --qhat;
        }

        // Store the quotient digit
        q[j] = qhat;
    } // D7: loop on j

    // D8: Unnormalize
    if (shift > 0) {
        rem.rightShift(shift);
    }

    quotient.normalize();
    rem.normalize();
    return rem;
}

mutable_bigint::uInt mutable_bigint::divaddLong(uInt dh, uInt dl, int_data &result, uInt offset) {
    uLong carry = 0;

    uLong sum = static_cast<uLong>(dl) + result[offset + 1ULL];
    result[offset + 1ULL] = static_cast<uInt>(sum);

    sum = static_cast<uLong>(dh) + result[offset] + carry;
    result[offset] = static_cast<uInt>(sum);
    carry = sum >> 32;
    return static_cast<uInt>(carry);
}

mutable_bigint::uInt mutable_bigint::mulsubLong(uInt dh, uInt dl, int_data &q, uInt x, uInt offset) {
    uLong xLong = x;
    offset += 2;
    uLong product = dl * xLong;
    uLong difference = q[offset] - product;
    q[offset--] = static_cast<uInt>(difference);
    uLong carry = (product >> 32) + ((static_cast<uInt>(difference) > static_cast<uInt>(~product)) ? 1 : 0);
    product = dh * xLong + carry;
    difference = q[offset] - product;
    q[offset--] = static_cast<uInt>(difference);
    carry = (product >> 32) + ((static_cast<uInt>(difference) > static_cast<uInt>(~product)) ? 1 : 0);
    return static_cast<uInt>(carry);
}

mutable_bigint::uLong mutable_bigint::divWord(uLong n, uInt d) {
    uLong dLong = d;
    uLong r, q;
    if (dLong == 1) {
        q = static_cast<uInt>(n);
        r = 0;
        return (r << 32) | static_cast<uInt>(q);
    }

    // Approximate the quotient and remainder
    q = (n >> 1) / (dLong >> 1);
    r = n - q * dLong;

    // Correct the approximation
    while (r < 0) {
        r += dLong;
        --q;
    }
    while (r >= dLong) {
        r -= dLong;
        ++q;
    }
    // n - q * dLong == r && 0 <= r < dLong, hence we're done
    return (r << 32) | static_cast<uInt>(q);
}

mutable_bigint mutable_bigint::mutableModInverse(const mutable_bigint &p) const {
    // Modulus is odd, use Schroeppel's algorithm
    if (p.isOdd()) {
        return modInverse(p);
    }

    // Base and modulus are even, throw exception
    if (isEven()) {;
        throw std::runtime_error("bigint is not invertible");
    }

    // Get even part of modulus expressed as a power of 2
    uInt powersOf2 = p.getLowestSetBit();

    // Construct odd part of modulus
    mutable_bigint oddMod = p;
    oddMod.rightShift(powersOf2);

    if (oddMod.isOne()) {
        return modInverseMP2(powersOf2);
    }

    // Calculate 1/a mod oddMod
    mutable_bigint oddPart = modInverse(oddMod);

    // Calculate 1/a mod evenMod
    mutable_bigint evenPart = modInverseMP2(powersOf2);

    // Combine the results using Chinese Remainder Theorem
    mutable_bigint y1 = modInverseBP2(oddMod, powersOf2);
    mutable_bigint y2 = oddMod.modInverseMP2(powersOf2);

    mutable_bigint temp1, temp2, result;

    oddPart.leftShift(powersOf2);
    oddPart.multiply(y1, result);

    evenPart.multiply(oddMod, temp1);
    temp1.multiply(y2, temp2);

    result.add(temp2);
    return result.divide(p, temp1);
}

// Calculate the multiplicative inverse of this mod 2^k.
mutable_bigint mutable_bigint::modInverseMP2(uInt k) const {
    if (isEven()) {
        throw std::runtime_error("bigint is not invertible (GCD != 1)");
    }

    if (k > 64) {
        return euclidModInverse(k);
    }

    Int t = inverseMod32(value[static_cast<uLong>(offset) + intLen - 1]);
    if (k < 33) {
        t = k == 32 ? t : (t & ((1ULL << k) - 1));
        return { static_cast<uInt>(t) };
    }

    uLong pLong = value[static_cast<uLong>(offset) + intLen - 1];
    if (intLen > 1) {
        pLong |= static_cast<uLong>(value[static_cast<uLong>(offset) + intLen - 2]) << 32;
    }
    uLong tLong = static_cast<uLong>(t);
    tLong *= (2 - pLong * tLong);
    tLong = k == 64 ? tLong : (tLong & ((1ULL << k) - 1));

    mutable_bigint result;
    result.value.resize(2);
    result.value[0] = static_cast<uInt>(tLong >> 32);
    result.value[1] = static_cast<uInt>(tLong);
    result.intLen = 2;
    result.normalize();
    return result;
}

// Returns the multiplicative inverse of val mod 2^32. Assumes val is odd.
mutable_bigint::Int mutable_bigint::inverseMod32(uInt val) {
    // Newton's iteration
    Int t = val;
    t *= 2 - val * t;
    t *= 2 - val * t;
    t *= 2 - val * t;
    t *= 2 - val * t;
    return t;
}

// Calculate the multiplicative inverse of 2^k mod mod, where mod is odd.
mutable_bigint mutable_bigint::modInverseBP2(const mutable_bigint &mod, uInt k) {
    // Copy the mod to protect original
    mutable_bigint a { 1 };
    mutable_bigint b { mod };
    return fixup(a, b, k);
}

mutable_bigint mutable_bigint::modInverse(const mutable_bigint &mod) const {
    mutable_bigint p = mod;
    mutable_bigint f = *this;
    mutable_bigint g = p;
    signed_mutable_bigint c { 1 };
    signed_mutable_bigint d;

    uInt k = 0;
    // Right shift f k times until odd, left shift d k times
    if (f.isEven()) {
        uInt trailingZeros = f.getLowestSetBit();
        f.rightShift(trailingZeros);
        d.leftShift(trailingZeros);
        k = trailingZeros;
    }

    // The Almost Inverse Algorithm
    while (!f.isOne()) {
        // If gcd(f, g) != 1, number is not invertible modulo mod
        if (f.isZero()) {
            throw std::runtime_error("bigint is not invertible");
        }
        // If f < g, exchange f, g and c, d
        if (f.compare(g) < 0) {
            mutable_bigint temp = f; f = g; g = temp;
            signed_mutable_bigint sTemp = d; d = c; c = sTemp;
        }

        // If f == g (mod 4)
        if (((f.value[static_cast<uLong>(f.offset) + f.intLen - 1]
            ^ g.value[static_cast<uLong>(g.offset) + g.intLen - 1]) & 3) == 0) {
            f.subtract(g);
            c.signedSubtract(d);
        }
        else {
            f.add(g);
            c.signedAdd(d);
        }

        // Right shiftf k times until odd, left shift d k times
        uInt trailingZeros = f.getLowestSetBit();
        f.rightShift(trailingZeros);
        d.leftShift(trailingZeros);
        k += trailingZeros;
    }

    while (c.sign < 0) {
        c.signedAdd(p);
    }

    return fixup(c, p, k);
}

// The Fixup Algorithm
// Calculates X such that X = C * 2^(-k) (mod P)
// Assumes C < P and P is odd
mutable_bigint mutable_bigint::fixup(mutable_bigint &c, mutable_bigint &p, uInt k) {
    mutable_bigint temp;
    // Set r to the multiplicative inverse of p mod 2 ^ 32
    Int r = -inverseMod32(p.value[static_cast<uLong>(p.offset) + p.intLen - 1]);
    for (Int i = 0, numWords = static_cast<Int>(k) >> 5; i < numWords; ++i) {
        // V = R * c (mod 2^j)
        Int v = r * c.value[static_cast<uLong>(c.offset) + c.intLen - 1];
        // c = c + (v * p)
        p.mul(v, temp);
        c.add(temp);
        // c = c / 2^j
        --c.intLen;
    }
    uInt numBits = k & 0x1F;
    if (numBits != 0) {
        // V = R * c (mod 2^j)
        Int v = r * c.value[static_cast<uLong>(c.offset) + c.intLen - 1];
        v &= ((1 << numBits) - 1);
        // c = c + (v * p)
        p.mul(v, temp);
        c.add(temp);
        // c = c / 2^j
        c.rightShift(numBits);
    }

    // In theory, c may be greater than p at this point (very rare!)
    while (c.compare(p) >= 0) {
        c.subtract(p);
    }

    return c;
}

// Uses the extended Euclidean algorithm to compute the modInverse of base
// mod a modulus that is a power of 2. The modulus is 2^k.
mutable_bigint mutable_bigint::euclidModInverse(uInt k) const {
    mutable_bigint b { 1 };
    b.leftShift(k);
    mutable_bigint mod = b;

    mutable_bigint a = *this;
    mutable_bigint q;
    mutable_bigint r = b.divide(a, q);

    // Swap b and r
    mutable_bigint swapper = b;
    b = r;
    r = swapper;

    mutable_bigint t1 = q;
    mutable_bigint t0 { 1 };
    mutable_bigint temp;

    while (!b.isOne()) {
        r = a.divide(b, q);

        if (r.intLen == 0) {
            throw std::runtime_error("bigint is not invertible");
        }

        swapper = r;
        a = swapper;

        if (q.intLen == 1) {
            t1.mul(q.value[q.offset], temp);
        }
        else {
            q.multiply(t1, temp);
        }
        swapper = q;
        q = temp;
        temp = swapper;
        t0.add(q);

        if (a.isOne()) {
            return t0;
        }

        r = b.divide(a, q);

        if (r.intLen == 0) {
            throw std::runtime_error("bigint is not invertible");
        }

        swapper = b;
        b = r;

        if (q.intLen == 1) {
            t0.mul(q.value[q.offset], temp);
        }
        else {
            q.multiply(t0, temp);
        }
        swapper = q; q = temp; temp = swapper;

        t1.add(q);
    }
    mod.subtract(t1);
    return mod;
}