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
// Relatively straightforward - do each pair of multiplies on the digits, and add
// them all together.
// ======================================================================================

// Num * Num
// Create a temp and then just call operator*=()
Num Num::operator*(const Num& rhs)
{
    Num temp{*this};
    return temp.operator*=(rhs);
}

// Num * Num
// Grow the lhs Num as needed
Num& Num::operator*=(const Num& rhs)
{
    // Even though this is lhs *= rhs, we need to move the lhs to a temp value because
    // we are going to write to parts of lhs constantly during the computation.
    Num multiplicand{*this};
    auto candbuf = multiplicand.cdatabuffer();
    auto plierbuf = rhs.cdatabuffer();

    // Now wipe out our lhs and resize it to hold the result - m digits from lhs, and
    // n digits from rhs.
    int m = multiplicand.data.len;
    int n = rhs.data.len;

    shrink(data.len);
    grow(m + n);
    auto lbuf = databuffer();

    // Multiply each digit of the multiplicand against the multiplier, adding the new
    // partial sum to the result.
    for (int j = 0; j < m; j++)
    {
        // Multiply each digit of the multiplier against this digit of the multiplicand,
        // add new digits to the result, and save the carry into the next digit.
        unsigned long long carry = 0;
        for (int i = 0; i < n; i++)
        {
            // This won't overflow:
            // (2^n-1)*(2^n-1) + (2^n-1) =
            // = 2^(2n) - 2*(2^n-1) + 1
            // = 2^(2n) - 2^n + 1
            // = 2^(2n-1) + 1 < 2^(2n)-1
            carry = carry + uint64_t(lbuf[i+j]) + uint64_t(candbuf[j]) * uint64_t(plierbuf[i]);
            lbuf[i+j] = (uint32_t) carry;
            carry >>= 32;
        }

        lbuf[j+n] = (uint32_t) carry;
    }

    // The sign of the result is the exclusive-or of the signs of the operands
    data.sign = (multiplicand.data.sign == rhs.data.sign) ? 0 : -1;

    // Now trim the result size down to its actual value, because
    // m+n was the max, not the actual size. We'll have to go at
    // most n places (e.g. n was 1)
    trim();

    return *this;
}

// Num * digit
// Create a temp and then just call operator*=()
Num Num::operator*(const uint32_t& rhs)
{
    Num temp{*this};
    return temp.operator*=(rhs);
}

// Num * digit
Num& Num::operator*=(const uint32_t& rhs)
{
    auto lbuf = databuffer();

    unsigned long long carry = 0;
    int i = 0;
    for (; i < data.len; i++)
    {
        carry = carry + lbuf[i] * uint64_t(rhs);
        lbuf[i] = (uint32_t) carry;
        carry >>= 32;
    }

    // If there is still a carry, we need to add another digit
    if (carry != 0)
    {
        lbuf = grow(1);
        lbuf[i] = (uint32_t) carry;
    }

    return *this;
}

// ======================================================================================
// Divide
//
// Use Knuth algorith implemented in MpDivide.cpp as MultiwordDivide.
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
    // If we are dividing into zero, it's always going to be zero
    if (data.len == 0)
        return *this;

    // Divide magnitudes
    // TBD temp space for quotient and remainder for long numbers
    Num quotient;
    Num remainder;
    int dividendSize = data.len;
    int divisorSize = rhs.data.len;
    quotient.resize(int16_t(dividendSize - divisorSize + 1));
    remainder.resize(int16_t(divisorSize));

    bool ok = MultiwordDivide<uint32_t>(
        quotient.databuffer(), remainder.databuffer(), databuffer(), rhs.cdatabuffer(), data.len, rhs.data.len);
    assert(ok);
    if (!ok)
        return *this; // this is not supposed to ever happen

    // Move the quotient into place as the result
    int qlen = data.len - rhs.data.len + 1;
    data.len = qlen; // TBD scan to make sure there are no leading zeros
    std::memcpy(databuffer(), quotient.databuffer(), data.len * sizeof(uint32_t));

    // Now trim the result size down to its actual value, because
    // m+n was the max, not the actual size. We'll have to go at
    // most m/2 places.
    trim();

    // The sign is positive if both signs are equal, otherwise negative
    data.sign = (data.sign == rhs.data.sign) ? 0 : -1;
    return *this;
}

// Num / uint32_t
// Create a temp and just call operator /=()
Num Num::operator/(const uint32_t& rhs)
{
    Num temp{*this};
    return temp.operator/=(rhs);
}

// Num /uint32_t
// TBD Note that this isn't an optimized version, it's just for symmetry. Fix it?
Num& Num::operator/=(const uint32_t& rhs)
{
    Num temp{rhs};
    return operator/=(temp);
}

// Do both divide and remainder at the same time
// TBD maybe we should return quotient? Or tuple of quotient, remainder?
void Num::divmod(const Num& rhs, Num& quotient, Num& remainder)
{
    // If dividend is zero, then quotient and remainder are both zero
    if (data.len == 0)
    {
        quotient.resize(0);
        remainder.resize(0);
        return;
    }

    // resize quotient and remainder as needed
    // these are max sizes, the real quotient and remainder could be smaller
    int dividendSize = len();
    int divisorSize = rhs.len();
    quotient.resize(int16_t(dividendSize - divisorSize + 1));
    remainder.resize(int16_t(divisorSize));

    bool ok = MultiwordDivide<uint32_t>(
        quotient.databuffer(), remainder.databuffer(), databuffer(), rhs.cdatabuffer(), data.len, rhs.data.len);
    assert(ok);
    if (!ok)
        return; // this is not supposed to ever happen

    quotient.trim();
    remainder.trim();
}

// Num / uint32_t
uint32_t Num::divmod(uint32_t rhs, Num& quotient)
{
    quotient.resize(data.len);

    auto buf = databuffer();
    auto qbuf = quotient.databuffer();

    uint32_t rem = 0;
    for (int i = len() - 1; i >= 0; --i)
    {
        uint32_t dig = uint32_t((rem * 0x1'0000'0000LL + buf[i]) / rhs);
        rem = uint64_t(buf[i]) - uint64_t(dig * rhs);
        qbuf[i] = dig;
    }

    quotient.trim();
    return rem;
}
