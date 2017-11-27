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

// Add a new initialized digit to Num
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
void Num::shrink()
{
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    // TBD real shrinking
    d.len -= 1;
    assert(d.len >= 0);
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
