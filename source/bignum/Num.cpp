// Num.cpp

#include "Num.h"

#include <stdint.h>

struct NumData
{
    int16_t len;
    int16_t sign;
    uint32_t data[7];
};

// Default constructor - create empty Num (has a value of zero)
Num::Num()
{
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    d.len = 0;
    d.sign = 0;
}

// Copy constructor (only used on new unconstructed object)
Num::Num(const Num& other) noexcept
{
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    const NumData& o{*reinterpret_cast<const NumData*>(other.raw)};

    d.len = o.len;
    d.sign = o.sign;
    for (int i = 0; i < d.len; i++)
        d.data[i] = o.data[i];
}

// Copy assignment operator (needs to free existing lhs)
Num& Num::operator=(const Num& other) noexcept
{
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
    uv = (uv >> 1) << 1; // lop off high-order bit

    const NumData& d{*reinterpret_cast<const NumData*>(raw)};
    long long v = d.sign ? -(long long)uv : uv;

    return v;
}

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
        {
            d.len += 1;
            d.data[i] = 0;
        }

        carry = d.data[i] + carry;
        d.data[i] = (uint32_t) carry;
        carry >>= 32;
    }

    return temp;
}

// Add two Nums together
Num Num::operator+(const Num& rhs)
{
    NumData& d{*reinterpret_cast<NumData*>(raw)};
    const NumData& o{*reinterpret_cast<const NumData*>(rhs.raw)};

    Num temp;
    NumData& t{*reinterpret_cast<NumData*>(temp.raw)};

    // TBD break this into multiple pieces so we don't need to check length
    // on every data fetch from this and rhs
    long long carry = 0;
    int N = d.len > o.len ? d.len : o.len;
    for (int i = 0; i < N || carry != 0; i++)
    {
        carry = carry + (i < d.len ? d.data[i] : 0) + (i < o.len ? o.data[i] : 0);
        t.len += 1;
        t.data[i] = (uint32_t) carry;
        carry >>= 32;
    }

    return temp;
}

int Num::len()
{
    const NumData& d{*reinterpret_cast<const NumData*>(raw)};
    return d.len;
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
