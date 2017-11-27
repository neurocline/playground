// ======================================================================================
// Num_addsub.cpp
//
// Add and subtract
// ======================================================================================

#include "Num.h"

#include <cassert>

// --------------------------------------------------------------------------------------

//#define VERBOSE_NUM
#ifdef VERBOSE_NUM
#include <iostream>
#define DIAG(TAG) std::cout << "Num " TAG "\n"
#else
#define DIAG(TAG)
#endif

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
// Grow the lhs Num as needed.
Num& Num::operator+=(const Num& rhs)
{
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    const NumData& s{*reinterpret_cast<const NumData*>(rhs.raw)};

    // If the signs are the same, we add and preserve the sign
    if (d.sign == s.sign)
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

Num& Num::addto(const Num& rhs)
{
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    const NumData& s{*reinterpret_cast<const NumData*>(rhs.raw)};

    // Two Num values can have different lengths. Add the prefix together, where
    // the prefix is the largest shared length between the two Nums. Note that by
    // definition one Num is all prefix and the other Num has an optional
    // suffix, and that the prefix may be zero length.
    int i = 0;
    int P = (d.len < s.len) ? d.len : s.len;
    long long carry = 0;

    for (; i < P; i++)
    {
        carry = carry + d.data[i] + s.data[i];
        d.data[i] = (uint32_t) carry;
        carry >>= 32;
    }

    // If the lhs has remaining data, then we just finish adding the carry into
    // the lhs until we have no more carry. This may result in increasing lhs by
    // 1 digit.
    if (d.len > i)
    {
        for (; carry != 0; i++)
        {
            if (i == d.len)
                grow(1);
            carry = carry + d.data[i];
            d.data[i] = (uint32_t) carry;
            carry >>= 32;
        }
    }

    // Otherwise, if the rhs has remaining data, then we add the carry and the rhs
    // together. This will grow the lhs to at least the size of the rhs, and it could
    // grow by 1 more if the carry+rhs spills over into a new digit. We won't know that
    // until we get to the end of the rhs+carry.
    else if (s.len > i)
    {
        grow(s.len - d.len);
        for (; i < s.len; i++)
        {
            carry = carry + s.data[i];
            d.data[i] = (uint32_t) carry;
            carry >>= 32;
        }
    }

    // If we still have a carry, create a new digit and place it there
    if (carry != 0)
    {
        grow(1);
        d.data[i] = (uint32_t) carry;
    }

    return *this;
}

// --------------------------------------------------------------------------------------

Num& Num::subfrom(const Num& /*rhs*/)
{
    assert(!"write me");
    return *this;
}

// --------------------------------------------------------------------------------------

// Add a single 'digit' to a Num
Num Num::operator+(const uint32_t& digit)
{
    Num temp{*this};
    NumData& d{*reinterpret_cast<NumData*>(temp.raw)};

    // Add through until no carry
    long long carry = digit;
    for (int i = 0; carry != 0; i++)
    {
        if (d.len == i)
            grow(1);

        carry = d.data[i] + carry;
        d.data[i] = (uint32_t) carry;
        carry >>= 32;
    }

    return temp;
}
