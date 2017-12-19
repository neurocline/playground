// ======================================================================================
// Num_power.cpp
//
// Exponentiation and others
//
// We use the ^ operator for exponentiation, since bitwise-xor of Nums is a very
// niche operation.
// ======================================================================================

#include "Num.h"

#include <cassert>

Num Num::operator^(const Num& rhs)
{
    Num temp{*this};
    return temp.operator^=(rhs);
}

Num& Num::operator^=(const Num& rhs)
{
    // there isn't enough memory to handle exponents this big
    if (rhs.data.len > 1 || rhs.cdatabuffer()[0] >= (1ULL << 31))
    {
        assert(!"can't handle");
        return *this;
    }

    uint32_t n = rhs.cdatabuffer()[0];
    Num y = 1;
    Num p = std::move(*this);

    for (; n != 0; p *= p)
    {
        if (n & 1)
            y = p * y;
        n >>= 1;
    }

    *this = std::move(y);
    return *this;
}

Num& Num::operator>>(const int rhs)
{
    Num temp{*this};
    return temp.operator>>=(rhs);
}

Num& Num::operator>>=(const int rhs)
{
    uint32_t* buf = databuffer();

    // If we have a trivial shift by zero, just return
    if (rhs == 0)
        return *this;

    // shift by digits first
    int shift = rhs & 0x1F;
    int dig = rhs >> 5;

    // If we shift past all the data, we just have zero remaining
    if (dig >= data.len)
    {
        data.len = 0;
        data.sign = 0;
        return *this;
    }

    // TBD combine both steps together
    // Shift whole digits
    for (int i = 0; i < dig; i++)
        buf[i] = buf[i+dig];
    data.len = data.len - dig;

    //  Now shift the bits themselves.
    for (int i = 0; i < data.len - 1; i++)
        buf[i] = (buf[i+1] << (32-shift)) | (buf[i] >> shift);
    buf[data.len - 1] >>= shift;

    // Remove extraneous leading zeros
    trim();

    return *this;
}
