// ======================================================================================
// Num_power.cpp
//
// Exponentiation and others
//
// We use the ^ operator for exponentiation, since bitwise-xor of Nums is a very
// niche operation.
// ======================================================================================

#include "Num.h"

#if 0

Num Num::operator^(const Num& rhs)
{
    Num temp{*this};
    return temp.operator^=(rhs);
}

Num& Num::operator^=(const Num& rhs)
{
    Num& x = *this;
    const Num& n = rhs;

    Num y = 1;
    Num p = x;
    #if 0
    while (true)
    {
        if (n & 1)
            y = p * y;
        n = n >> 1;
        if (n == 0)
            break;
        p = p * p;
    }
    #endif

    for (; n != 0; p *= p)
    {
        if (n & 1)
            y = p * y;
        n >>= 1;
    }

    return y;
}
#endif
