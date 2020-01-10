#include "BigInt.h"
#include "MutableBigInt.h"

std::vector<bigint::uInt> bigint::digitsPerLong;
std::vector<bigint> bigint::longRadix;
std::vector<bigint::uInt> bigint::digitsPerInt;
std::vector<bigint::uInt> bigint::intRadix;

bigint::static_initializer_1 bigint::_init1;

std::vector<bigint> bigint::posConst;
std::vector<bigint> bigint::negConst;
std::vector<std::vector<bigint>> bigint::powerCache;
std::vector<double> bigint::logCache;
const double bigint::LOG_TWO = std::log(2.0);

bigint::static_initializer_2 bigint::_init2;

const bigint bigint::ZERO { { }, 0 };
const bigint bigint::ONE = bigint::valueOf(1ULL);
const bigint bigint::TWO = bigint::valueOf(2ULL);
const bigint bigint::NEGATIVE_ONE = bigint::valueOf(-1LL);

bigint::bigint() {
    sign = 0;
}

bigint::bigint(const bigint &val) {
    mag = val.mag;
    sign = val.sign;
}

bigint::bigint(const int_data &mag, Int sign) {
    this->sign = mag.size() == 0 ? 0 : sign;
    this->mag = mag;
    if (mag.size() >= MAX_MAG_LENGTH) {
        checkRange();
    }
}

bigint bigint::valueOf(Long val) {
    if (val == 0) {
        return ZERO;
    }
    if (val > 0 && val <= MAX_CONSTANT) {
        return posConst[val];
    }
    if (val < 0 && val >= -MAX_CONSTANT) {
        return negConst[-val];
    }
    return { val };
}

bigint bigint::valueOf(uLong val) {
    if (val == 0) {
        return ZERO;
    }
    if (val > 0 && val <= MAX_CONSTANT) {
        return posConst[val];
    }
    return { val };
}

bigint::bigint(Int val) : bigint(static_cast<Long>(val)) { }
bigint::bigint(uInt val) : bigint(static_cast<uLong>(val)) { }

bigint::bigint(Long val) {
    if (val == 0) {
        *this = ZERO;
        return;
    }
    else if (val > 0 && val <= MAX_CONSTANT) {
        *this = posConst[val];
        return;
    }
    else if (val < 0) {
        if (val >= -MAX_CONSTANT) {
            *this = negConst[-val];
            return;
        }
        val = -val;
        sign = -1;
    }
    else {
        sign = 1;
    }

    uInt highWord = static_cast<uInt>(static_cast<uLong>(val) >> 32);
    if (highWord == 0) {
        mag.resize(1);
        mag[0] = static_cast<uInt>(val);
    }
    else {
        mag.resize(2);
        mag[0] = highWord;
        mag[1] = static_cast<uInt>(val);
    }
}

bigint::bigint(uLong val) {
    if (val == 0) {
        *this = ZERO;
    }
    else if (val > 0 && val <= MAX_CONSTANT) {
        *this = posConst[val];
    }
    else {
        sign = 1;
        
        uInt highWord = static_cast<uInt>(val >> 32);
        if (highWord == 0) {
            mag.resize(1);
            mag[0] = static_cast<uInt>(val);
        }
        else {
            mag.resize(2);
            mag[0] = highWord;
            mag[1] = static_cast<uInt>(val);
        }
    }
}

void bigint::checkRange() const {
    if (mag.size() > MAX_MAG_LENGTH || mag.size() == MAX_MAG_LENGTH && mag[0] > static_cast<uInt>(std::numeric_limits<Int>::max())) {
        reportOverflow();
    }
}

void bigint::reportOverflow() {
    throw std::overflow_error("BigInteger will overflow supported range.");
}

// Multiply x vector times word y in place, and add word z
void bigint::destructiveMulAdd(int_data &x, uInt y, uInt z) {
    // Perform the multiplication word by word
    uLong ylong = y;
    uLong zlong = z;
    Long length = x.size();

    uLong product = 0;
    uLong carry = 0;
    for (Long i = length - 1; i >= 0; --i) {
        product = ylong * x[i] + carry;
        x[i] = static_cast<uInt>(product);
        carry = product >> 32;
    }

    // Perform the addition
    Long sum = x[length - 1] + zlong;
    x[length - 1] = static_cast<uInt>(sum);
    carry = Integer::unsignedRightShift(sum, 32);
    for (Long i = length - 1; i >= 0; --i) {
        sum = Integer::unsign(x[i]) + carry;
        x[i] = static_cast<uInt>(sum);
        carry = Integer::unsignedRightShift(sum, 32);
    }
}

// Multiply an array by one word k and add to result, return the carry
bigint::uInt bigint::mulAdd(int_data &out, const int_data &in, uInt offset, uInt len, uInt k) {
    uLong kLong = k;
    uLong carry = 0;

    offset = static_cast<uInt>(out.size() - offset - 1);
    for (Int j = len - 1; j >= 0; --j) {
        uLong product = in[j] * kLong + out[offset] + carry;
        out[offset--] = static_cast<uInt>(product);
        carry = product >> 32;
    }
    return static_cast<uInt>(carry);
}

// Add one word to the number a mlen words into a. Return the resulting carry.
bigint::uInt bigint::addOne(int_data &a, uInt offset, uInt mlen, uInt carry) {
    offset = static_cast<uInt>(a.size() - 1 - mlen - offset);
    uLong t = static_cast<uLong>(a[offset]) + carry;

    a[offset] = static_cast<uInt>(t);
    if ((t >> 32) == 0) {
        return 0;
    }
    while (--mlen >= 0) {
        // Carry out of number
        if (--offset < 0) {
            return 1;
        }
        else {
            ++a[offset];
            if (a[offset] != 0) {
                return 0;
            }
        }
    }
    return 1;
}

bigint::Int bigint::compare(const bigint &val) const {
    if (sign == val.sign) {
        switch (sign) {
            case 1:
                return compareMagnitude(val);
            case -1:
                return val.compareMagnitude(*this);
            default:
                return 0;
        }
    }
    return sign > val.sign ? 1 : -1;
}

bigint::Int bigint::compareMagnitude(const bigint &val) const {
    uLong len1 = mag.size();
    uLong len2 = val.mag.size();
    if (len1 < len2) {
        return -1;
    }
    if (len1 > len2) {
        return 1;
    }
    for (uInt i = 0; i < len1; ++i) {
        uInt a = mag[i];
        uInt b = val.mag[i];
        if (a != b) {
            return a < b ? -1 : 1;
        }
    }
    return 0;
}

bigint::int_data bigint::trustedStripLeadingZeroInts(const int_data &val) {
    uLong vlen = val.size();
    uInt keep;
    for (keep = 0; keep < vlen && val[keep] == 0; ++keep);
    if (keep == 0) {
        return val;
    }
    else {
        int_data out;
        std::copy(val.begin() + keep, val.end(), std::back_inserter(out));
        return out;
    }
}

void bigint::increment(int_data &val) {
    uInt lastSum = 0;
    for (Int i = static_cast<Int>(val.size() - 1); i >= 0 && lastSum == 0; --i) {
        lastSum = val[i] += 1;
    }
    if (lastSum == 0) {
        val.clear();
        val.insert(val.begin(), 1);
    }
}

bigint bigint::getRadixConversionCache(uInt radix, uInt exponent) {
    std::vector<bigint> &cacheLine = powerCache[radix];
    if (exponent < cacheLine.size()) {
        return cacheLine[exponent];
    }

    uLong oldLength = cacheLine.size();
    cacheLine.resize(static_cast<uLong>(exponent) + 1);
    for (uLong i = oldLength; i <= exponent; ++i) {
        cacheLine[i] = cacheLine[i - 1].pow(2);
    }

    powerCache[radix] = cacheLine;
    return cacheLine[exponent];
}

bigint::uInt bigint::intLength() const {
    return (bitLength() >> 5) + 1;
}

bigint::uInt bigint::signBit() const {
    return sign < 0 ? 1 : 0;
}

bigint::Int bigint::signInt() const {
    return sign < 0 ? -1 : 0;
}

bigint::uInt bigint::getInt(Int n) const {
    if (n < 0) {
        return 0;
    }
    if (n >= mag.size()) {
        return signInt();
    }

    Int magInt = static_cast<Int>(mag[mag.size() - n - 1]);
    return sign >= 0 ? magInt : n <= firstNonzeroIntNum() ? -magInt : ~magInt;
}

bigint::Int bigint::firstNonzeroIntNum() const {
    // Search for the first nonzero int
    Int i;
    Int mlen = static_cast<Int>(mag.size());
    for (i = mlen - 1; i >= 0 && mag[i] == 0; --i);
    return mlen - i - 1;
}

bigint::uInt bigint::bitLengthForInt(uInt x) {
    return 32 - Integer::numberOfLeadingZeros(x);
}

void bigint::primitiveRightShift(int_data &a, uInt len, uInt n) {
    uInt n2 = 32 - n;
    for (uInt i = len - 1, c = a[i]; i > 0; --i) {
        uInt b = c;
        c = a[i - 1];
        a[i] = (c << n2) | (b >> n);
    }
    a[0] >>= n;
}

void bigint::primitiveLeftShift(int_data &a, uInt len, uInt n) {
    if (len == 0 || n == 0) {
        return;
    }

    uInt n2 = 32 - n;
    for (uInt i = 0, c = a[i], m = i + len - 1; i < m; ++i) {
        uInt b = c;
        c = a[i + 1ULL];
        a[i] = (b << n) | (c >> n2);
    }
    a[len - 1] <<= n;
}

bigint::uInt bigint::bitLength() const {
    uLong len = mag.size();
    if (len == 0) {
        return 0;
    }
    else {
        // Calculate the bit length of the magnitude
        uInt magBitLength = static_cast<uInt>(((len - 1) << 5) + bitLengthForInt(mag[0]));
        if (sign < 0) {
            // Check if magnitude is power of 2
            bool pow2 = Integer::bitCount(mag[0]) == 1;
            for (Int i = 1; i < len && pow2; ++i) {
                pow2 = mag[i] == 0;
            }
            return pow2 ? magBitLength - 1 : magBitLength;
        }
        else {
            return magBitLength;
        }
    }
}

bigint::Int bigint::getLowestSetBit() const {
    Int lsb = 0;
    if (sign == 0) {
        lsb -= 1;
    }
    else {
        // Search for lowest order nonzero int
        uInt i, b;
        for (i = 0; (b = getInt(i)) == 0; ++i);
        lsb += (i << 5) + Integer::numberOfTrailingZeros(b);
    }
    return lsb;
}

bigint bigint::add(const bigint &val) const {
    if (val.sign == 0) {
        return *this;
    }
    if (sign == 0) {
        return val;
    }
    if (val.sign == sign) {
        return { add(mag, val.mag), sign };
    }

    auto cmp = compareMagnitude(val);
    if (cmp == 0) {
        return ZERO;
    }
    int_data resultMag = (cmp > 0 ? subtract(mag, val.mag) : subtract(val.mag, mag));
    resultMag = trustedStripLeadingZeroInts(resultMag);
    return { resultMag, cmp == sign ? 1 : -1 };
}

// Adds the contents of x and value val
// Assumes x.size() > 0, val >= 0
bigint::int_data bigint::add(const int_data &x, uLong val) {
    uLong sum = 0;
    uLong xIndex = x.size();
    int_data result;
    uInt highWord = static_cast<uInt>(val >> 32);
    if (highWord == 0) {
        result.resize(xIndex);
        sum = x[--xIndex] + val;
        result[xIndex] = static_cast<uInt>(sum);
    }
    else {
        if (xIndex == 1) {
            result.resize(2);
            sum = val + x[0];
            result[1] = static_cast<uInt>(sum);
            result[0] = static_cast<uInt>(sum >> 32);
            return result;
        }
        else {
            result.resize(xIndex);
            sum = x[--xIndex] + val;
            result[xIndex] = static_cast<uInt>(sum);
            sum = x[--xIndex] + static_cast<uLong>(highWord) + (sum >> 32);
            result[xIndex] = static_cast<uInt>(sum);
        }
    }
    // Copy remainder of longer number while carry propagation is required
    bool carry = (sum >> 32) != 0;
    while (xIndex > 0 && carry) {
        --xIndex;
        carry = ((result[xIndex] = x[xIndex] + 1) == 0);
    }
    // Copy remainder of longer number
    while (xIndex > 0) {
        --xIndex;
        result[xIndex] = x[xIndex];
    }
    // Grow result if necessary
    if (carry) {
        result.insert(result.begin(), 0x01);
    }
    return result;
}

// Adds the contents of the x and y
bigint::int_data bigint::add(const int_data &x, const int_data &y) {
    // If x is shorter, swap the two
    if (x.size() < y.size()) {
        return add(y, x);
    }

    uLong xIndex = x.size();
    uLong yIndex = y.size();
    int_data result(xIndex);
    uLong sum = 0;
    if (yIndex == 1) {
        sum = static_cast<uLong>(x[--xIndex]) + y[0];
        result[xIndex] = static_cast<uInt>(sum);
    }
    else {
        // Add common parts of both numbers
        while (yIndex > 0) {
            sum = static_cast<uLong>(x[--xIndex]) + y[--yIndex] + (sum >> 32);
            result[xIndex] = static_cast<uInt>(sum);
        }
    }
    // Copy remainder of longer number while carry propagation is required
    bool carry = (sum >> 32) != 0;
    while (xIndex > 0 && carry) {
        --xIndex;
        carry = (result[xIndex] = x[xIndex] + 1) == 0;
    }
    // Copy remainder of longer number
    while (xIndex > 0) {
        --xIndex;
        result[xIndex] = x[xIndex];
    }
    // Grow result if necessary
    if (carry) {
        result.insert(result.begin(), 0x01);
    }
    return result;
}

bigint bigint::subtract(const bigint &val) const {
    if (val.sign == 0) {
        return *this;
    }
    if (sign == 0) {
        return val.negate();
    }
    if (val.sign != sign) {
        return { add(mag, val.mag), sign };
    }

    auto cmp = compareMagnitude(val);
    if (cmp == 0) {
        return ZERO;
    }
    int_data resultMag = (cmp > 0 ? subtract(mag, val.mag) : subtract(val.mag, mag));
    resultMag = trustedStripLeadingZeroInts(resultMag);
    return { resultMag, cmp == sign ? 1 : -1 };
}

bigint::int_data bigint::subtract(uLong val, const int_data &little) {
    uInt highWord = static_cast<uInt>(val >> 32);
    if (highWord == 0) {
        return { static_cast<uInt>(val - little[0]) };
    }
    else {
        int_data result(2);
        if (little.size() == 1) {
            uLong difference = val - little[0];
            result[1] = static_cast<uInt>(difference);
            // Subtract remainder of longer number while borrow propagates
            bool borrow = (static_cast<Long>(difference) >> 32) != 0;
            if (borrow) {
                result[0] = highWord - 1;
            }
            else {
                // Copy remainder of longer number
                result[0] = highWord;
            }
            return result;
        }
        else {
            uLong difference = val - little[1];
            result[1] = static_cast<uInt>(difference);
            difference = static_cast<uLong>(highWord) - little[0] + (static_cast<Long>(difference) >> 32);
            result[0] = static_cast<Int>(difference);
            return result;
        }
    }
}

// Subtracts the contents of the second argument from the first
// big must represent a larger number than val
bigint::int_data bigint::subtract(const int_data &big, uLong val) {
    uInt highWord = static_cast<uInt>(val >> 32);
    uLong bigIndex = big.size();
    int_data result(bigIndex);
    uLong difference = 0;

    if (highWord == 0) {
        difference = big[--bigIndex] - val;
        result[bigIndex] = static_cast<uInt>(difference);
    }
    else {
        difference = big[--bigIndex] - val;
        result[bigIndex] = static_cast<uInt>(difference);
        difference = static_cast<uLong>(big[--bigIndex]) - highWord + (static_cast<Long>(difference) >> 32);
        result[bigIndex] = static_cast<uInt>(difference);
    }
    // Subtract remainder of longer number while borrow propagates
    bool borrow = (static_cast<Long>(difference) >> 32) != 0;
    while (bigIndex > 0 && borrow) {
        --bigIndex;
        borrow = ((result[bigIndex] = big[bigIndex] - 1) == -1);
    }
    // Copy remainder of longer number
    while (bigIndex > 0) {
        --bigIndex;
        result[bigIndex] = big[bigIndex];
    }
    return result;
}

// Subtracts the contents of the little from big
// big must represent a larger number than little
bigint::int_data bigint::subtract(const int_data &big, const int_data &little) {
    uLong bigIndex = big.size();
    int_data result(bigIndex);
    uLong littleIndex = little.size();
    uLong difference = 0;

    // Subtract common parts of both numbers
    while (littleIndex > 0) {
        difference = (static_cast<uLong>(big[--bigIndex]) - static_cast<uLong>(little[--littleIndex])) + (static_cast<Long>(difference) >> 32);
        result[bigIndex] = static_cast<uInt>(difference);
    }
    // Subtract remainder of longer number while borrow propagates
    bool borrow = (static_cast<Long>(difference) >> 32) != 0;
    while (bigIndex > 0 && borrow) {
        --bigIndex;
        borrow = ((result[bigIndex] = big[bigIndex] - 1) == -1);
    }
    // Copy remainder of longer number
    while (bigIndex > 0) {
        --bigIndex;
        result[bigIndex] = big[bigIndex];
    }
    return result;
}

bigint bigint::multiply(const bigint &val) const {
    if (val.sign == 0 || sign == 0) {
        return ZERO;
    }

    uLong xlen = mag.size();
    uLong ylen = val.mag.size();

    if ((xlen < KARATSUBA_THRESHOLD) || (ylen < KARATSUBA_THRESHOLD)) {
        Int resultSign = sign == val.sign ? 1 : -1;
        if (ylen == 1) {
            return multiplyByInt(mag, val.mag[0], resultSign);
        }
        if (xlen == 1) {
            return multiplyByInt(val.mag, mag[0], resultSign);
        }
        int_data result;
        multiplyToLen(mag, xlen, val.mag, ylen, result);
        result = trustedStripLeadingZeroInts(result);
        return { result, resultSign };
    }
    else {
        if ((xlen < TOOM_COOK_THRESHOLD) || (ylen < TOOM_COOK_THRESHOLD)) {
            return multiplyKaratsuba(*this, val);
        }
        else {
            return multiplyToomCook3(*this, val);
        }
    }
}

bigint bigint::multiplyByInt(const int_data &x, uInt y, Int sign) {
    if (Integer::bitCount(y) == 1) {
        return { shiftLeft(x, Integer::numberOfTrailingZeros(y)), sign };
    }
    uLong xlen = x.size();
    int_data rmag(xlen + 1);
    uLong carry = 0;
    uLong yl = static_cast<uLong>(y);
    uInt rstart = static_cast<uInt>(rmag.size() - 1);
    for (Int i = static_cast<Int>(xlen - 1); i >= 0; --i) {
        uLong product = x[i] * yl + carry;
        rmag[rstart--] = static_cast<uInt>(product);
        carry = product >> 32;
    }
    if (carry == 0) {
        rmag.insert(rmag.begin(), 0);
    }
    else {
        rmag[rstart] = static_cast<uInt>(carry);
    }
    return { rmag, sign };
}

void bigint::multiplyToLen(const int_data &x, uLong xlen, const int_data &y, uLong ylen, int_data &z) {
    uInt xstart = static_cast<uInt>(xlen - 1);
    uInt ystart = static_cast<uInt>(ylen - 1);

    z.resize(xlen + ylen);

    uLong carry = 0;
    for (Int j = ystart, k = ystart + xstart + 1; j >= 0; --j, --k) {
        uLong product = static_cast<uLong>(y[j]) * x[xstart] + carry;
        z[k] = static_cast<uInt>(product);
        carry = product >> 32;
    }
    z[xstart] = static_cast<uInt>(carry);

    for (Int i = xstart - 1; i >= 0; --i) {
        carry = 0;
        for (Int j = ystart, k = ystart + i + 1; j >= 0; --j, --k) {
            uLong product = static_cast<uLong>(y[j]) * x[i] + static_cast<uLong>(z[k]) + carry;
            z[k] = static_cast<uInt>(product);
            carry = product >> 32;
        }
        z[i] = static_cast<uInt>(carry);
    }
}

bigint bigint::multiplyKaratsuba(const bigint &x, const bigint &y) {
    uInt xlen = static_cast<uInt>(x.mag.size());
    uInt ylen = static_cast<uInt>(y.mag.size());

    // The number of ints in each half of the number
    uInt half = (std::max(xlen, ylen) + 1) / 2;

    // xl and yl are the lower halves of x and y respectively
    // xh and yh are the uppser halves
    bigint xl = x.getLower(half);
    bigint xh = x.getUpper(half);
    bigint yl = y.getLower(half);
    bigint yh = y.getUpper(half);

    bigint p1 = xh.multiply(yh);
    bigint p2 = xl.multiply(yl);

    // p3 = (xh + xl) * (yh + yl)
    bigint p3 = xh.add(xl).multiply(yh.add(yl));

    // result = p1 * 2 ^ (32 * 2 * half) + (p3 - p1  - p2) * 2 ^ (32 * half) + p2
    bigint result = p1.shiftLeft(32 * half).add(p3.subtract(p1).subtract(p2)).shiftLeft(32 * half).add(p2);

    return x.sign != y.sign ? result.negate() : result;
}

bigint bigint::multiplyToomCook3(const bigint &a, const bigint &b) {
    uInt alen = static_cast<uInt>(a.mag.size());
    uInt blen = static_cast<uInt>(b.mag.size());

    uInt largest = std::max(alen, blen);

    // k is the size (in ints) of the lower-order slices
    uInt k = (largest + 2) / 3;

    // r is the size (in ints) of the highest-order slice
    uInt r = largest - 2 * k;

    // Obtain slices of the numbers. a2 and b2 are the most significant
    // bits of the numbers a and b, and a0 and b0 the least significant.
    bigint a0, a1, a2, b0, b1, b2;
    a2 = a.getToomSlice(k, r, 0, largest);
    a1 = a.getToomSlice(k, r, 1, largest);
    a0 = a.getToomSlice(k, r, 2, largest);
    b2 = b.getToomSlice(k, r, 0, largest);
    b1 = b.getToomSlice(k, r, 1, largest);
    b0 = b.getToomSlice(k, r, 2, largest);

    bigint v0, v1, v2, vm1, vinf, t1, t2, tm1, da1, db1;

    v0 = a0.multiply(b0);
    da1 = a2.add(a0);
    db1 = b2.add(b0);
    vm1 = da1.subtract(a1).multiply(db1.subtract(b1));
    da1 = da1.add(a1);
    db1 = db1.add(b1);
    v1 = da1.multiply(db1);
    v2 = da1.add(a2).shiftLeft(1).subtract(a0).multiply(
        db1.add(b2).shiftLeft(1).subtract(b0));
    vinf = a2.multiply(b2);

    // The algorithm requires two divisions by 2 and one by 3.
    // All divisions are known to be exact, that is, they do not produce
    // remainders, and all results are positive. The divisions by 2 are
    // implemented as right shifts which are relatively efficient, leaving
    // only an exact division by 3, which is done by a specialized
    // linear-time algorithm.
    t2 = v2.subtract(vm1).exactDivideBy3();
    tm1 = v1.subtract(vm1).shiftRight(1);
    t1 = v1.subtract(v0);
    t2 = t2.subtract(t1).shiftRight(1);
    t1 = t1.subtract(tm1).subtract(vinf);
    t2 = t2.subtract(vinf.shiftLeft(1));
    tm1 = tm1.subtract(t2);

    // Number of bits to shift left
    uInt ss = k * 32;

    bigint result = vinf.shiftLeft(ss).add(t2).shiftLeft(ss).add(t1).shiftLeft(ss)
        .add(tm1).shiftLeft(ss).add(v0);

    return a.sign != b.sign ? result.negate() : result;
}

bigint bigint::getToomSlice(uInt lowerSize, uInt upperSize, uInt slice, uInt fullsize) const {
    Int start, end, sliceSize, len, offset;

    len = static_cast<uInt>(mag.size());
    offset = fullsize - len;

    if (slice == 0) {
        start = 0 - offset;
        end = upperSize - 1 - offset;
    }
    else {
        start = upperSize + (slice - 1) * lowerSize - offset;
        end = start + lowerSize - 1;
    }

    if (start < 0) {
        start = 0;
    }
    if (end < 0) {
        return ZERO;
    }

    sliceSize = (end - start) + 1;

    if (sliceSize <= 0) {
        return ZERO;
    }

    // While performing Toom-Cook, all slices are positive and
    // the sign is adjsuted when the final number is composed.
    if (start == 0 && sliceSize >= len) {
        return abs();
    }

    int_data intSlice(sliceSize);
    std::copy(mag.begin() + start, mag.begin() + start + sliceSize, intSlice.begin());
    return { trustedStripLeadingZeroInts(intSlice), 1 };
}

bigint bigint::exactDivideBy3() const {
    uLong len = mag.size();
    int_data result(len);
    Long x, w, q, borrow;
    borrow = 0;
    for (Int i = static_cast<Int>(len - 1); i >= 0; --i) {
        x = mag[i];
        w = x - borrow;
        borrow = borrow > x ? 1 : 0;

        // 0xAAAAAAAB is the modular inverse of 3 (mod 2^32). Thus,
        // the effect of this is to divide by 3 (mod 2^32).
        // This is much faster than division on most architectures.
        q = static_cast<uInt>(w * 0xAAAAAAABLL);
        result[i] = static_cast<uInt>(q);

        // Now check the borrow. The second check can of course be
        // eliminated if the first fails.
        if (q >= 0x55555556LL) {
            ++borrow;
            if (q >= 0xAAAAAAABLL) {
                ++borrow;
            }
        }
    }
    return { trustedStripLeadingZeroInts(result), sign };
}

bigint bigint::divide(const bigint &val) const {
    return divideAndRemainder(val).first;
}

bigint bigint::remainder(const bigint &val) const {
    return divideAndRemainder(val).second;
}

bigint bigint::mod(const bigint &val) const {
    bigint r = remainder(val);
    return r.sign < 0 ? val + r : r;
}

std::pair<bigint, bigint> bigint::divideAndRemainder(const bigint &val) const {
    if (val.mag.size() < BURNIKEL_ZIEGLER_THRESHOLD || (mag.size() - val.mag.size()) < BURNIKEL_ZIEGLER_OFFSET) {
        mutable_bigint q, a { mag }, b { val.mag };
        mutable_bigint r = a.divideKnuth(b, q);
        return { q.toBigint(sign * val.sign), r.toBigint(sign) };
    }
    else {
        mutable_bigint q, a { *this }, b { val };
        mutable_bigint r = a.divideAndRemainderBurnikelZiegler(b, q);
        bigint qb = q.isZero() ? ZERO : q.toBigint(sign * val.sign);
        bigint rb = r.isZero() ? ZERO : r.toBigint(sign);
        return { qb, rb };
    }
}


bigint bigint::shiftLeft(uInt n) const {
    if (sign == 0) {
        return ZERO;
    }
    if (n > 0) {
        return { shiftLeft(mag, n), sign };
    }
    if (n == 0) {
        return *this;
    }
    return shiftRightImpl(Integer::negate(n));
}

bigint::int_data bigint::shiftLeft(const int_data &mag, uInt n) {
    uInt nInts = n >> 5;
    uInt nBits = n & 0x1F;
    uLong magLen = mag.size();
    int_data newMag;

    if (nBits == 0) {
        newMag = mag;
        newMag.resize(magLen + nInts);
    }
    else {
        uInt i = 0;
        uInt nBits2 = 32 - nBits;
        uInt highBits = mag[0] >> nBits2;
        if (highBits != 0) {
            newMag.resize(magLen + nInts + 1);
            newMag[i++] = highBits;
        }
        else {
            newMag.resize(magLen + nInts);
        }
        uInt j = 0;
        while (j < magLen - 1) {
            newMag[i++] = mag[j++] << nBits | mag[j] >> nBits2;
        }
        newMag[i] = mag[j] << nBits;
    }
    return newMag;
}

bigint bigint::shiftRight(uInt n) const {
    if (sign == 0) {
        return ZERO;
    }
    if (n > 0) {
        return shiftRightImpl(n);
    }
    if (n == 0) {
        return *this;
    }
    return { shiftLeft(mag, Integer::negate(n)), sign };
}

bigint bigint::shiftRightImpl(uInt n) const {
    uInt nInts = n >> 5;
    uInt nBits = n & 0x1F;
    uLong magLen = mag.size();
    int_data newMag;

    // Special case: entire contents shifted off the end
    if (nInts >= magLen) {
        return sign >= 0 ? ZERO : negConst[1];
    }

    if (nBits == 0) {
        uInt newMagLen = static_cast<uInt>(magLen - nInts);
        std::copy(mag.begin(), mag.begin() + newMagLen, std::back_inserter(newMag));
    }
    else {
        uInt i = 0;
        uInt highBits = mag[0] >> nBits;
        if (highBits != 0) {
            newMag.resize(magLen - nInts);
            newMag[i++] = highBits;
        }
        else {
            newMag.resize(magLen - nInts - 1);
        }

        uInt nBits2 = 32 - nBits;
        uInt j = 0;
        while (j < magLen - nInts - 1) {
            newMag[i++] = (mag[j++] << nBits2) | mag[j] >> nBits;
        }
    }

    if (sign < 0) {
        // Find out whether any one-bits were shifted off the end
        bool onesLost = false;
        for (uInt i = static_cast<uInt>(magLen - 1), j = static_cast<uInt>(magLen - nInts); i >= j && !onesLost; --i) {
            onesLost = mag[i] != 0;
        }
        if (!onesLost && nBits != 0) {
            onesLost = mag[magLen - nInts - 1] << (32 - nBits) != 0;
        }
        if (onesLost) {
            increment(newMag);
        }
    }
    return { newMag, sign };
}

bigint bigint::abs() const {
    return { mag, sign * sign };
}

bigint bigint::negate() const {
    return { mag, -sign };
}

bigint bigint::pow(uInt exponent) const {
    if (sign == 0) {
        return exponent == 0 ? ONE : *this;
    }

    bigint partToSquare = abs();

    // Factor our powers of two from the base, as the exponentiation of
    // these can be done by left shifts only.
    // The remaining part can then be exponentiated faster. The powers
    // of two will be multiplied back at the end.
    Int powersOfTwo = partToSquare.getLowestSetBit();
    Long bitsToShiftLong = static_cast<Long>(powersOfTwo) * exponent;
    if (bitsToShiftLong > std::numeric_limits<Int>::max()) {
        reportOverflow();
    }
    uInt bitsToShift = static_cast<uInt>(bitsToShiftLong);

    uInt remainingBits;

    // Factor the powers of two out quickly by shifting right, if needed
    if (powersOfTwo > 0) {
        partToSquare = partToSquare.shiftRight(powersOfTwo);
        remainingBits = partToSquare.bitLength();
        // Nothing left but +/- 1?
        if (remainingBits == 1) {
            if (sign < 0 && (exponent & 1) == 1) {
                return NEGATIVE_ONE.shiftLeft(bitsToShift);
            }
            else {
                return ONE.shiftLeft(bitsToShift);
            }
        }
    }
    else {
        remainingBits = partToSquare.bitLength();
        // Nothing left but +/- 1?
        if (remainingBits == 1) {
            if (sign < 0 && (exponent & 1) == 1) {
                return NEGATIVE_ONE;
            }
            else {
                return ONE;
            }
        }
    }

    // This is a quick way to approximate the size of the result,
    // similar to doing log2[n] * exponent. This will give an upper bound
    // of how big the result can be, and which algorithm to use.
    uLong scaleFactor = static_cast<uLong>(remainingBits) * exponent;

    // Use slightly different algorithms for small and large operands.
    // See if the result will safely fit into a Long. (Largest 2^63 - 1)
    if (partToSquare.mag.size() == 1 && scaleFactor <= 62) {
        // Small number algorithm. Everything fits into a Long.
        Int newSign = (sign < 0 && (exponent & 1) == 1) ? -1 : 1;
        uLong result = 1;
        uLong baseToPow2 = partToSquare.mag[0];

        uInt workingExponent = exponent;

        // Perform exponentiation using repeated squaring trick
        while (workingExponent != 0) {
            if ((workingExponent & 1) == 1) {
                result *= baseToPow2;
            }

            if ((workingExponent >>= 1) != 0) {
                baseToPow2 *= baseToPow2;
            }
        }

        // Multiply back the powers of two (quickly, by shifting left)
        if (powersOfTwo > 0) {
            // Fits in Long?
            if (bitsToShift + scaleFactor <= 62) {
                return valueOf((result << bitsToShift) * newSign);
            }
            else {
                return valueOf(result * newSign).shiftLeft(bitsToShift);
            }
        }
        else {
            return valueOf(result * newSign);
        }
    }
    else {
        if (static_cast<uLong>(bitLength()) * exponent / 32 > MAX_MAG_LENGTH) {
            reportOverflow();
        }
        // Large number algorithm. This is basically identical to
        // the algorithm above, but called multiply() and square()
        // which may use more efficient algorithms for large numbers.
        bigint answer = ONE;

        uInt workingExponent = exponent;
        // Perform exponentiation using repreated squaring trick
        while (workingExponent != 0) {
            if ((workingExponent & 1) == 1) {
                answer = answer.multiply(partToSquare);
            }

            if ((workingExponent >>= 1) != 0) {
                partToSquare = partToSquare.square();
            }
        }
        // Multiply back the (exponentiated) powers of two (quickly,
        // by shifting left)
        if (powersOfTwo > 0) {
            answer = answer.shiftLeft(bitsToShift);
        }

        if (sign < 0 && (exponent & 1) == 1) {
            return answer.negate();
        }
        else {
            return answer;
        }
    }
}

bigint bigint::square() const {
    if (sign == 0) {
        return ZERO;
    }
    uInt len = static_cast<uInt>(mag.size());
    if (len < KARATSUBA_SQUARE_THRESHOLD) {
        int_data z;
        squareToLen(mag, len, z);
        return { trustedStripLeadingZeroInts(z), 1 };
    }
    else {
        if (len < TOOM_COOK_SQUARE_THRESHOLD) {
            return squareKaratsuba();
        }
        else {
            return squareToomCook3();
        }
    }
}

void bigint::squareToLen(const int_data &x, uInt len, int_data &z) {
    // The algorithm used here is apadted from Colin Plumb's C library.
    uInt zlen = len << 1;
    if (z.size() < zlen) {
        z.resize(zlen);
    }

    // Store the squares, right shifted one bit (i.e., divided by 2)
    uInt lastProductLowWord = 0;
    for (uInt j = 0, i = 0; j < len; ++j) {
        uLong piece = x[j];
        uLong product = piece * piece;
        z[i++] = (lastProductLowWord << 31) | static_cast<uInt>(product >> 33);
        z[i++] = static_cast<uInt>(product >> 1);
        lastProductLowWord = static_cast<uInt>(product);
    }

    // Add in off-diagonal sums
    for (uInt i = len, offset = 1; i > 0; --i, offset += 2) {
        uInt t = x[i - 1];
        t = mulAdd(z, x, offset, i - 1, t);
        addOne(z, offset - 1, i, t);
    }

    // Shift back up and set low bit
    primitiveLeftShift(z, zlen, 1);
    z[zlen - 1] |= x[len - 1] & 1;
}

// Uses the Karatsuba squaring method
bigint bigint::squareKaratsuba() const {
    uInt half = static_cast<uInt>((mag.size() + 1) / 2);
    bigint xl = getLower(half);
    bigint xh = getUpper(half);
    bigint xhs = xh.square();
    bigint xls = xl.square();

    // xh^2 << 64 + (((xl + xh)^2 - (xh^2 + xl^2)) << 32) + xl^2
    return xhs.shiftLeft(half * 32)
        .add(xl.add(xh).square().subtract(xhs.add(xls)))
        .shiftLeft(half * 32).add(xls);
}

// Uses the 3-way Toom-Cook squaring algorithm
bigint bigint::squareToomCook3() const {
    uInt len = static_cast<uInt>(mag.size());

    // k is the size (in ints) of the lower-order slices.
    uInt k = (len + 2) / 3;

    // r is the size (in ints) of the highest order slice.
    uInt r = len - 2 * k;

    // Obtain slices of the numbers. a2 is the most significant
    // bits of the number, and a0 the least significant.
    bigint a0, a1, a2;
    a2 = getToomSlice(k, r, 0, len);
    a1 = getToomSlice(k, r, 1, len);
    a0 = getToomSlice(k, r, 2, len);
    bigint v0, v1, v2, vm1, vinf, t1, t2, tm1, da1;

    v0 = a0.square();
    da1 = a2.add(a0);
    vm1 = da1.subtract(a1).square();
    da1 = da1.add(a1);
    v1 = da1.square();
    vinf = a2.square();
    v2 = da1.add(a2).shiftLeft(1).subtract(a0).square();

    // The algorithm requires two divisions by 2 and one by 3.
    // All divisions are known to be exact, that is, they do not produce
    // remainders, and all results are positive. The divisions by 2 are
    // implemented as right shifts which are relatively efficient, leaving
    // only a division by 3.
    // The division by 3 is done by an optimized algorithm for this case.
    t2 = v2.subtract(vm1).exactDivideBy3();
    tm1 = v1.subtract(vm1).shiftRight(1);
    t1 = v1.subtract(v0);
    t2 = t2.subtract(t1).shiftRight(1);
    t1 = t1.subtract(tm1).subtract(vinf);
    t2 = t2.subtract(vinf.shiftLeft(1));
    tm1 = tm1.subtract(t2);

    // Number of bits to shift left.
    uInt ss = k * 32;

    return vinf.shiftLeft(ss).add(t2).shiftLeft(ss).add(t1).shiftLeft(ss).add(tm1).shiftLeft(ss).add(v0);
}

bigint bigint::getLower(uInt n) const {
    uLong len = mag.size();

    if (len <= n) {
        return abs();
    }

    int_data lowerInts(n);
    std::copy(mag.begin() + (len - n), mag.end(), lowerInts.begin());
    return { trustedStripLeadingZeroInts(lowerInts), 1 };
}

bigint bigint::getUpper(uInt n) const {
    uLong len = mag.size();

    if (len <= n) {
        return ZERO;
    }

    uInt upperLen = static_cast<uInt>(len - n);
    int_data upperInts(upperLen);
    std::copy(mag.begin(), mag.begin() + upperLen, upperInts.begin());
    return { trustedStripLeadingZeroInts(upperInts), 1 };
}

bigint bigint::modInverse(const bigint &m) const {
    if (m.sign != 1) {
        throw std::invalid_argument("bigint modulus not positive");
    }
    if (m == ONE) {
        return ZERO;
    }

    // Calculate (this mod m)
    bigint modVal = *this;
    if (sign < 0 || compareMagnitude(m) >= 0) {
        modVal = mod(m);
    }
    if (modVal == ONE) {
        return ONE;
    }

    mutable_bigint a { modVal };
    mutable_bigint b { m };
    
    mutable_bigint result = a.mutableModInverse(b);
    return result.toBigint(1);
}

bigint::uLong bigint::toULong() const {
    uLong result = 0;

    for (Int i = 1; i >= 0; --i) {
        result = (result << 32) + getInt(i);
    }
    return result;
}

std::string bigint::toString() const {
    return toString(10);
}

std::string bigint::toString(uInt radix) const {
    if (sign == 0) {
        return "0";
    }
    if (radix < MIN_RADIX || radix > MAX_RADIX) {
        radix = 10;
    }

    // If it's small enough, use smallToString
    if (mag.size() <= SCHOENHAGE_BASE_CONVERSION_THRESHOLD) {
        return smallToString(radix);
    }

    // Use recursive toString, which requires positive arguments
    std::string str;
    if (sign < 0) {
        toString(negate(), str, radix, 0);
        str.insert(str.begin(), '-');
    }
    else {
        toString(*this, str, radix, 0);
    }
    return str;
}

// Used to perform toString when arguments are small
std::string bigint::smallToString(uInt radix) const {
    if (sign == 0) {
        return "0";
    }

    // Compute upper bound on number of digit groups and allocate space
    uInt maxNumDigitGroups = static_cast<uInt>((4 * mag.size() + 6) / 7);
    std::vector<std::string> digitGroup(maxNumDigitGroups);
    // char buffer[65];

    // Translate number to string, a digit group at a time
    bigint tmp = abs();
    uInt numGroups = 0;
    while (tmp.sign != 0) {
        bigint &d = longRadix[radix];
        mutable_bigint q, a { tmp.mag }, b { d.mag };
        mutable_bigint r = a.divide(b, q);
        bigint q2 = q.toBigint(tmp.sign * d.sign);
        bigint r2 = r.toBigint(tmp.sign * d.sign);

        // Radix is currently unused
        // Write parsing function and use here
        digitGroup[numGroups++] = std::to_string(r2.toULong());
        tmp = q2;
    }

    // Put sign (if any) and first digit group into result buffer
    std::stringstream buf;
    if (sign < 0) {
        buf << '-';
    }
    buf << digitGroup[numGroups - 1];

    // Append remaining digit groups padded with leading zeros
    for (Int i = numGroups - 2; i >= 0; --i) {
        // Prepend (any) leading zeros for this digit group
        uInt numLeadingZeros = static_cast<uInt>(digitsPerLong[radix] - digitGroup[i].size());
        if (numLeadingZeros != 0) {
            buf << std::string(numLeadingZeros, '0');
        }
        buf << digitGroup[i];
    }
    return buf.str();
}

void bigint::toString(const bigint &u, std::string &str, uInt radix, Int digits) {
    // If we're smaller than a certain threshold, use the smallToString
    // method, padding with leading zeros when necessary.
    if (u.mag.size() <= SCHOENHAGE_BASE_CONVERSION_THRESHOLD) {
        std::string s = u.smallToString(radix);
        // Pad with internal zeros if necessary
        // Don't pad if we're at the beginning of the string
        if ((s.length() < digits) && (str.length() > 0)) {
            str.append(std::string(digits - s.length(), '0'));
        }
        str.append(s);
        return;
    }
    uInt b = u.bitLength();

    // Calculate a value for n in the equation radix^(2^n) = u
    // and subtract 1 from that value. This is used to find the cache
    // index that contaisn the best value to divide u.
    uInt n = static_cast<uInt>(std::round(std::log(b * LOG_TWO / logCache[radix]) / LOG_TWO - 1.0));
    bigint v = getRadixConversionCache(radix, n);
    const auto [q, r] = u.divideAndRemainder(v);

    uInt expectedDigits = 1 << n;

    // Now recursively build the two halves of each number.
    toString(q, str, radix, digits - expectedDigits);
    toString(r, str, radix, expectedDigits);
}

bigint bigint::operator+(const bigint &val) const {
    return add(val);
}

bigint bigint::operator-(const bigint &val) const {
    return subtract(val);
}

bigint bigint::operator*(const bigint &val) const {
    return multiply(val);
}

bigint bigint::operator/(const bigint &val) const {
    return divide(val);
}

bigint bigint::operator%(const bigint &val) const {
    return remainder(val);
}

bigint bigint::operator<<(uInt val) const {
    return shiftLeft(val);
}

bigint bigint::operator>>(uInt val) const {
    return shiftRight(val);
}

bigint bigint::operator+() const {
    return *this;
}

bigint bigint::operator-() const {
    return negate();
}

bigint &bigint::operator+=(const bigint &val) {
    *this = add(val);
    return *this;
}

bigint &bigint::operator-=(const bigint &val) {
    *this = subtract(val);
    return *this;
}

bigint &bigint::operator*=(const bigint &val) {
    *this = multiply(val);
    return *this;
}

bigint &bigint::operator/=(const bigint &val) {
    *this = divide(val);
    return *this;
}

bigint &bigint::operator%=(const bigint &val) {
    *this = remainder(val);
    return *this;
}

bigint &bigint::operator<<=(uInt val) {
    *this = shiftLeft(val);
    return *this;
}

bigint &bigint::operator>>=(uInt val) {
    *this = shiftRight(val);
    return *this;
}

bool bigint::operator<(const bigint &val) const {
    return compare(val) < 0;
}

bool bigint::operator==(const bigint &val) const {
    return compare(val) == 0;
}

bool bigint::operator!=(const bigint &val) const {
    return compare(val) != 0;
}

bool bigint::operator>(const bigint &val) const {
    return compare(val) > 0;
}

bool bigint::operator<=(const bigint &val) const {
    return compare(val) <= 0;
}

bool bigint::operator>=(const bigint &val) const {
    return compare(val) >= 0;
}

bigint::operator std::string() const {
    return toString();
}

std::ostream &operator<<(std::ostream &out, const bigint &val) {
    out << val.toString();
    return out;
}