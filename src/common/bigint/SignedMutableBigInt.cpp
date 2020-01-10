#include "SignedMutableBigInt.h"

signed_mutable_bigint::signed_mutable_bigint() 
    : mutable_bigint() { }

signed_mutable_bigint::signed_mutable_bigint(uInt val) 
    : mutable_bigint(val) { }

signed_mutable_bigint::signed_mutable_bigint(const mutable_bigint &val) 
    : mutable_bigint(val) { }

void signed_mutable_bigint::signedAdd(const signed_mutable_bigint &addend) {
    if (sign == addend.sign) {
        add(addend);
    }
    else {
        sign = sign * subtract(addend);
    }
}

void signed_mutable_bigint::signedAdd(const mutable_bigint &addend) {
    if (sign == 1) {
        add(addend);
    }
    else {
        sign = sign * subtract(addend);
    }
}

void signed_mutable_bigint::signedSubtract(const signed_mutable_bigint &addend) {
    if (sign == addend.sign) {
        sign = sign * subtract(addend);
    }
    else {
        add(addend);
    }
}

void signed_mutable_bigint::signedSubtract(const mutable_bigint &addend) {
    if (sign == 1) {
        sign = sign * subtract(addend);
    }
    else {
        add(addend);
    }
    if (intLen == 0) {
        sign = 1;
    }
}