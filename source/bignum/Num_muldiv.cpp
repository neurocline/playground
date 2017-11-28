// ======================================================================================
// Num_muldiv.cpp
//
// Multiply and divide
// ======================================================================================

#include "Num.h"

#include <cassert>
#include <cstring>

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

    // The sign is positive if both signs are equal, otherwise negative
    d.sign = (d.sign == s.sign) ? 0 : -1;
    return *this;
}
