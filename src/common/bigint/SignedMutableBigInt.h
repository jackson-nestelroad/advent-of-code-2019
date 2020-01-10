#pragma once

#include "MutableBigInt.h"

class signed_mutable_bigint : mutable_bigint {
private:
    Int sign = 1;

    signed_mutable_bigint();
    signed_mutable_bigint(uInt val);
    signed_mutable_bigint(const mutable_bigint &val);

    void signedAdd(const signed_mutable_bigint &addend);
    void signedAdd(const mutable_bigint &addend);
    void signedSubtract(const signed_mutable_bigint &addend);
    void signedSubtract(const mutable_bigint &addend);

    friend class mutable_bigint;
};