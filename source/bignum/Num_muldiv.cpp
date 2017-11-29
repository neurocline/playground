// ======================================================================================
// Num_muldiv.cpp
//
// Multiply and divide
// ======================================================================================

#include "Num.h"

#include <cassert>
#include <cstring>

// ======================================================================================
// Multiply
//
// Relatively straightforward.
// ======================================================================================

// Num * Num
// Create a temp and then just call operator*=()
Num Num::operator*(const Num& rhs)
{
    Num temp{*this};
    return temp.operator*=(rhs);
}

// Num / Num
// Grow the lhs Num as needed
Num& Num::operator*=(const Num& rhs)
{
    // Move lhs to temp so that we can intialize the result. The
    // result has m+n digits
    Num multiplicand{*this};
    shrink(int16_t(len()));
    grow(int16_t(multiplicand.len() + rhs.len()));

    NumData& d{*reinterpret_cast<NumData*>(raw)};
    const NumData& m{*reinterpret_cast<const NumData*>(multiplicand.raw)};
    const NumData& n{*reinterpret_cast<const NumData*>(rhs.raw)};

    for (int j = 0; j < m.len; j++)
    {
        unsigned long long carry = 0;
        for (int i = 0; i < n.len; i++)
        {
            // This won't overflow:
            // (2^n-1)*(2^n-1) + (2^n-1) =
            // = 2^(2n) - 2*(2^n-1) + 1
            // = 2^(2n) - 2^n + 1
            // = 2^(2n-1) + 1 < 2^(2n)-1
            carry = uint64_t(m.data[i]) * uint64_t(n.data[j]) + uint64_t(d.data[i+j]) + carry;
            d.data[i+j] = (uint32_t) carry;
            carry >>= 32;
        }

        d.data[j+m.len] = (uint32_t) carry;
    }

    // The sign of the result is the exclusive-or of the signs of the operands
    d.sign = (m.sign == n.sign) ? 0 : -1;

    // Now trim the result size down to its actual value, because
    // m+n was the max, not the actual size. We'll have to go at
    // most m/2 places.
    trim();

    return *this;
}

void Num::trim()
{
    NumData& d{*reinterpret_cast<NumData*>(raw)};

    int16_t i = d.len;
    for (; i > 0; i--)
        if (d.data[i-1] != 0)
            break;
    if (i != d.len)
        shrink(d.len - i);
}

// ======================================================================================
// Divide
//
// Use Knuth algorith implmented in MpDivide.cpp as MultiwordDivide.
// ======================================================================================

// Num / Num
// Create a temp and then just call operator/=()
Num Num::operator/(const Num& rhs)
{
    Num temp{*this};
    return temp.operator/=(rhs);
}

// Num / Num
// Grow/shrink the lhs Num as needed.
Num& Num::operator/=(const Num& rhs)
{
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    const NumData& s{*reinterpret_cast<const NumData*>(rhs.raw)};

    // If we are dividing into zero, it's always going to be zero
    if (d.len == 0)
        return *this;

    // Divide magnitudes
    // TBD temp space for quotient and remainder for long numbers
    uint32_t quotient[7];
    uint32_t remainder[7];

    bool ok = MultiwordDivide<uint32_t>(quotient, remainder, d.data, s.data, d.len, s.len);
    assert(ok);
    if (!ok)
        return *this; // this is not supposed to ever happen

    int16_t qlen = d.len - s.len + 1;
    d.len = qlen; // TBD scan to make sure there are no leading zeros
    std::memcpy(d.data, quotient, d.len * sizeof(uint32_t));

    // Now trim the result size down to its actual value, because
    // m+n was the max, not the actual size. We'll have to go at
    // most m/2 places.
    trim();

    // The sign is positive if both signs are equal, otherwise negative
    d.sign = (d.sign == s.sign) ? 0 : -1;
    return *this;
}

void Num::divmod(const Num& rhs, Num& quotient, Num& remainder)
{
    // resize quotient and remainder as needed
    // these are max sizes, the real quotient and remainder could be smaller
    int dividendSize = len();
    int divisorSize = rhs.len();
    quotient.resize(int16_t(dividendSize - divisorSize + 1));
    remainder.resize(int16_t(divisorSize));

    NumData& q{*reinterpret_cast<NumData*>(quotient.raw)};
    NumData& r{*reinterpret_cast<NumData*>(remainder.raw)};

    NumData& d{*reinterpret_cast<NumData*>(raw)};
    const NumData& s{*reinterpret_cast<const NumData*>(rhs.raw)};

    bool ok = MultiwordDivide<uint32_t>(q.data, r.data, d.data, s.data, d.len, s.len);
    assert(ok);
    if (!ok)
        return; // this is not supposed to ever happen
}
