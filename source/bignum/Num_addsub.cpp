// ======================================================================================
// Num_addsub.cpp
//
// Add and subtract
// ======================================================================================

#include "Num.h"

#include <cassert>

// ======================================================================================
// Addition
//
// We store numbers as sign,magnitude instead of as signed numbers, to make some
// operations easier. It also makes it a little easier to work with arbitrary precision.
//
// This complicates addition a little. We have four cases for adding numbers together
// while respecting signs and operating on magnitudes (which only have positive values).
//
// +a + +b = +(|a| + |b|)
// +a + -b = |a| >= |b| ? +(|a| - |b|) : -(|b| - |a|)
// -a + +b = |a| >= |b| ? -(|a| - |b|) : +(|b| - |a|)
// -a + -b = -(|a| + |b|)
//
// Or, if we write our numbers as a tuple (s,m) where s=sign and m=magnitude, we have
// this
//
// a.s==b.s: r.s = a.s, r.m = a.m + b.m
// a.m>=b.m: r.s = a.s, r.m = a.m - b.m
// a.m<b.m: r.s = b.s, r.m = b.m - a.m
//
// To see this is true, consider the cases. Obviously if we add two positive numbers,
// we get another positive number. And if we add two negative numbers, this is the same
// as adding the absolute values together and then negating. So we just add magnitudes
// and preserve the sign.
//
// If signs differ, then we will end up with a positive number if and only if the magnitude
// of the positive number is greater than the magnitude of the negative number; otherwise
// we get a negative number. Since addition is commutative, +a + -b is the same as -b + a,
// so we can just rename variables and merge operations
//
// If the two signs are the same, then we just add magnitudes and preserve the sign.
//
// If the two signs differ, we subtract the smaller magnitude from the larger magnitude
// and preserve the sign of the larger magnitude.
// ======================================================================================

// Num + Num
// Create a temp and then just call operator+=()
Num Num::operator+(const Num& rhs)
{
    Num temp{*this};
    return temp.operator+=(rhs);
}

// Num += Num
// Grow/shrink the lhs Num as needed.
Num& Num::operator+=(const Num& rhs)
{
    // If the signs are the same, we add and preserve the sign
    if (data.sign == rhs.data.sign)
        return addto(rhs);

    // If the signs are different, we are actually subtracting. If the lhs has the larger
    // magnitude, we can just subtract rhs from lhs while preserving sign of lhs.
    if (magcmp(rhs) >= 0)
        return subfrom(rhs);

    // The signs are different, and the rhs is the large value. We need a temp to operate
    // on. Copy lhs to a temp, copy rhs to lhs, and then subtract magnitudes.
    Num temp{*this};
    *this = rhs;
    return subfrom(temp);
}

// --------------------------------------------------------------------------------------

// Num + digit
// Add a single 'digit' to a Num
#if 0
Num Num::operator+(const uint32_t& digit)
{
    Num temp{*this};
    return temp.operator+=(digit);
}

// Num += digit
// Note that this is mainly a hack at the moment, since it assumes lhs is non-negative
Num& Num::operator+=(const uint32_t& digit)
{
    assert(data.sign == 0);

    // Add through the Num; we can stop once we no longer have a carry
    int i = 0;
    long long carry = digit;
    uint32_t* buf = databuffer();
    for (; i < data.len && carry != 0; i++)
    {
        carry = carry + buf[i];
        buf[i] = (uint32_t) carry;
        carry >>= 32;
    }

    // If we still have a carry, we need to grow the Num by a digit
    if (carry != 0)
    {
        buf = grow(1);
        buf[i] = (uint32_t) carry;
    }

    return *this;
}
#endif

// --------------------------------------------------------------------------------------

Num& Num::addto(const Num& rhs)
{
    auto lbuf = databuffer();
    auto rbuf = rhs.cdatabuffer();

    // Two Num values can have different lengths. Add the prefix together, where
    // the prefix is the largest shared length between the two Nums. Note that by
    // definition one Num is all prefix and the other Num has an optional
    // suffix, and that the prefix may be zero length.
    int i = 0;
    int P = (data.len < rhs.data.len) ? data.len : rhs.data.len;
    long long carry = 0;

    for (; i < P; i++)
    {
        carry = carry + lbuf[i] + rbuf[i];
        lbuf[i] = (uint32_t) carry;
        carry >>= 32; // this is either +1 or 0
    }

    // If the lhs has remaining data, then we just finish adding the carry into
    // the lhs until we have no more carry. This may result in increasing lhs by
    // 1 digit.
    if (data.len > i)
    {
        for (; carry != 0; i++)
        {
            if (i == data.len)
                lbuf = grow(1);
            carry = carry + lbuf[i];
            lbuf[i] = (uint32_t) carry;
            carry >>= 32;
        }
    }

    // Otherwise, if the rhs has remaining data, then we add the carry and the rhs
    // together. This will grow the lhs to at least the size of the rhs, and it could
    // grow by 1 more if the carry+rhs spills over into a new digit. We won't know that
    // until we get to the end of the rhs+carry.
    else if (rhs.data.len > i)
    {
        grow(rhs.data.len - data.len);
        for (; i < rhs.data.len; i++)
        {
            carry = carry + rbuf[i];
            lbuf[i] = (uint32_t) carry;
            carry >>= 32;
        }
    }

    // If we still have a carry, create a new digit and place it there
    if (carry != 0)
    {
        lbuf = grow(1);
        lbuf[i] = (uint32_t) carry;
    }

    return *this;
}

// --------------------------------------------------------------------------------------

// compute magnitude-only Num - Num, where lhs is guaranteed to be
// bigger than rhs, so that we don't have underflow. This simplifies the logic
// for subtract.
Num& Num::subfrom(const Num& rhs)
{
    auto lbuf = databuffer();
    auto rbuf = rhs.cdatabuffer();

    // Subtract the prefix - we already know that |lhs| > |rhs|
    int i = 0;
    long long borrow = 0;

    for (; i < rhs.data.len; i++)
    {
        borrow = borrow + lbuf[i] - rbuf[i];
        if (borrow < 0)
            lbuf[i] = (uint32_t)((1LL<<32) + borrow);
        else
            lbuf[i] = (uint32_t) borrow;
        borrow >>= 32; // this is either -1 or 0
    }

    // If we have a borrow left, ripple through the remaining number
    for (; i < data.len && borrow != 0; i++)
    {
        borrow = borrow + lbuf[i];
        lbuf[i] = (borrow < 0) ? (uint32_t)((1LL<<32) + borrow) : (uint32_t) borrow;
        borrow >>= 32; // this is either -1 or 0
    }

    // Trim so MSB is non-zero
    trim();

    return *this;
}

// ======================================================================================
// Subtraction
//
// Subtraction is almost exactly like addition, except that we flip the sign of the rhs
// before adding the two numbers together. This is because addition already has to handle
// the case of subtraction (e.g. adding a negative number to a positive number).
//
// +a - +b = +a + -b
// +a - -b = +a + +b
// -a - +b = -a + -b
// -a - -b = -a + +b
//
// and we know how to do to the additions from above, so we have
//
// +a + -b = |a| >= |b| ? +(|a| - |b|) : -(|b| - |a|)
// +a + +b = +(|a| + |b|)
// -a + -b = -(|a| + |b|)
// -a + +b = |a| >= |b| ? -(|a| - |b|) : +(|b| - |a|)

// For subtraction, we add if the signs differ, and keep the sign of the lhs; if the
// signs are the same, we subtract the smaller amount from the greater amount, and keep
// the sign of the greater amount.
//
// a.s!=b.s: r.s = a.s, r.m = a.m + b.m
// a.m>=b.m: r.s = a.s, r.m = a.m - b.m
// a.m<b.m: r.s = b.s, r.m = b.m - a.m

// ======================================================================================

// Num - Num
// Create a temp and then just call operator-=()
Num Num::operator-(const Num& rhs)
{
    Num temp{*this};
    return temp.operator-=(rhs);
}

// Num -= Num
// Grow/shrink the lhs Num as needed.
Num& Num::operator-=(const Num& rhs)
{
    // If the signs are different, we add and preserve the sign
    if (data.sign != rhs.data.sign)
        return addto(rhs);

    // If the signs are the same, we are actually subtracting. If the lhs has the larger
    // magnitude, we can just subtract rhs from lhs while preserving sign of lhs.
    if (magcmp(rhs) >= 0)
        return subfrom(rhs);

    // The signs are the same, and the rhs is the large value. We need a temp to operate
    // on. Copy lhs to a temp, copy rhs to lhs, and then subtract magnitudes.
    Num temp{*this};
    *this = rhs;
    return subfrom(temp);
}

// --------------------------------------------------------------------------------------

// Num - digit
// Subtract a single 'digit' from a Num
#if 0
Num Num::operator-(const uint32_t& digit)
{
    Num temp{*this};
    return temp.operator-=(digit);
}

// Num -= digit
Num& Num::operator-=(const uint32_t& digit)
{
    // for now, just cheat and do it the slow way, because we may not actually
    // do this in the log run (subtracting signed vs unsigned is odd).
    return operator-=(Num{digit});
}
#endif
