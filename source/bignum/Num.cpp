// ======================================================================================
// Num.cpp
// - arbitrary-precision numbers with basic operator support
//
// Basic Num support - constructors/destructors, and serialization
// ======================================================================================

#include "Num.h"

#include <cassert>
#include <cstdint>

// --------------------------------------------------------------------------------------

//#define VERBOSE_NUM
#ifdef VERBOSE_NUM
#include <iostream>
#define DIAG(TAG) std::cout << "Num " TAG "\n"
#else
#define DIAG(TAG)
#endif

// ======================================================================================
// Constructors
// ======================================================================================

// Default constructor - create empty Num (has a value of zero)
Num::Num()
{
    DIAG("default constructor");
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    d.len = 0;
    d.sign = 0;
}

// Copy constructor (only used on new unconstructed object)
Num::Num(const Num& other) noexcept
{
    DIAG("copy constructor");
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    const NumData& o{*reinterpret_cast<const NumData*>(other.raw)};

    d.len = o.len;
    d.sign = o.sign;
    for (int i = 0; i < d.len; i++)
        d.data[i] = o.data[i];
}

// Copy assignment operator (needs to free existing lhs data first)
Num& Num::operator=(const Num& other) noexcept
{
    DIAG("copy assignment operator");
    if (this != &other)
    {
        // Free existing Num data

        // Copy other num
        NumData& d{*reinterpret_cast<NumData*>(raw)};
        const NumData& o{*reinterpret_cast<const NumData*>(other.raw)};

        d.len = o.len;
        d.sign = o.sign;
        for (int i = 0; i < d.len; i++)
            d.data[i] = o.data[i];
    }

    return *this;
}

Num::~Num()
{
    // TBD free allocated memory
}

// initialize a Num from a long long value. Pick the shortest Num
// that holds the value.
Num::Num(long long v)
{
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    d.len = 0;

    d.sign = 0;
    unsigned long long uv = v;
    if (v < 0)
    {
        d.sign = -1;
        uv = (~uv + 1);
    }

    for (int i = 0; uv != 0; i++)
    {
        d.len += 1;
        d.data[i] = (uv & 0xFFFFFFFFL);
        uv >>= 32;
    }
}

// ======================================================================================
// Conversions
// ======================================================================================

// Convert a Num to an unsigned long long value (modulo 2^64)
uint64_t Num::to_uint64() const
{
    const NumData& d{*reinterpret_cast<const NumData*>(raw)};
    uint64_t uv = 0;
    int len = d.len <= 2 ? d.len : 2;
    for (int i = len - 1; i >= 0; i--)
        uv = (uv << 32) | d.data[i];

    return uv;
}

// Convert a Num to a long long value (modulo 2^63)
int64_t Num::to_int64() const
{
    uint64_t uv = to_uint64();
    uv = (uv << 1) >> 1; // lop off high-order bit

    const NumData& d{*reinterpret_cast<const NumData*>(raw)};
    long long v = d.sign ? -(long long)uv : uv;

    return v;
}

// Convert Num to zero-terminated string.
// Return size of buffer required
int Num::to_cstring(char* p, int buflen, int base)
{
    int i = 0;
    #define PUT(c) { if (i < buflen) p[i] = c; i++; }

    // Now convert the number
    Num t{*this};
    NumData& dd{*reinterpret_cast<NumData*>(&t)};
    bool neg = dd.sign != 0;
    if (neg)
        PUT('-')
    dd.sign = 0;

    Num B{base};
    Num quotient;
    Num remainder;

    do
    {
        //t.divmod(B, quotient, remainder);
        //PUT('0' + (char) remainder.to_int64())
        uint32_t r = t.divmod(base, quotient);
        PUT('0' + (char) r);
        t = quotient; // replace with pointer swap
        t.trim();
    } while (t.len() != 0);

    PUT(0)

    // now reverse the string
    int j = neg ? 1 : 0; // skip any leading '-' sign
    int k = i - 2; // leave zero-terminator in place
    while (j < k)
    {
        char c = p[j]; p[j] = p[k]; p[k] = c;
        j++;
        k--;
    }
    return i;
    #undef PUT
}

// ======================================================================================
// Utility
// ======================================================================================

// Resize Num
void Num::resize(int16_t size)
{
    if (size == len())
        return;
    else if (size < len())
        shrink(int16_t(len()) - size);
    else
        grow(size - int16_t(len()));
}

// Add new initialized digit to Num
// TBD maybe we shouldn't initialize here, since most users will overwrite new storage?
void Num::grow(int16_t amt)
{
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    // TBD real growing
    assert(d.len + amt <= sizeof(d.data)/sizeof(d.data[0]));
    for (int i = d.len; i < d.len + amt; i++)
        d.data[i] = 0;
    d.len += amt;
}

// Remove the most significant digit from Num (assumed to be zero)
void Num::shrink(int16_t amt)
{
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    // TBD real shrinking
    assert(d.len - amt >= 0);
    d.len -= amt;

    // if we made it zero-size, make the sign positive as a convenience
    if (d.len == 0)
        d.sign = 0;
}

// Resize Num so that the highest digit is non-zero
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

// Num <=> Num
//   -1: Num < Num
//    0: Num == Num
//   +1: Num > Num
int Num::magcmp(const Num& rhs)
{
    // Trivially, if the numbers are different lengths, the longer number is
    // greater than the shorter number
    if (len() != rhs.len())
        return len() > rhs.len() ? 1 : -1;

    // Compare magnitude digits from greater to lesser
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    const NumData& s{*reinterpret_cast<const NumData*>(rhs.raw)};
    for (int i = d.len - 1; i >= 0; --i)
    {
        if (d.data[i] != s.data[i])
            return d.data[i] > s.data[i] ? 1 : -1;
    }

    // Numbers have the same magnitude
    return 0;
}

// Num <=> digit
// question - what does it mean to comparing a signed Num vs an unsigned digit?
// maybe this is actually nonsense.
int Num::magcmp(const uint32_t& digit)
{
    // Check edge cases first
    if (len() > 1)
        return 1;
    if (len() == 0 && digit == 0)
        return 0;

    NumData& d{*reinterpret_cast<NumData*>(raw)};
    return d.data[0] > digit ? 1 : d.data[0] < digit ? -1 : 0;
}

int Num::len() const
{
    const NumData& d{*reinterpret_cast<const NumData*>(raw)};
    return d.len;
}

int Num::sign() const
{
    const NumData& d{*reinterpret_cast<const NumData*>(raw)};
    return d.sign;
}

// Read from underlying data array
uint32_t Num::operator[](int i) const
{
    const NumData& d{*reinterpret_cast<const NumData*>(raw)};
    return d.data[i];
}

uint32_t& Num::operator [](int i)
{
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    return d.data[i];
}

// TBD
// - use std::copy to copy arrays
// - add allocation for large Num values
