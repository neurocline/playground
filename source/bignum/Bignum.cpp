// Bignum.cpp

#include <stdint.h>
#include <cstring>

// Visual Studio specific
#include <intrin.h>

// Note - Intel has a 128-bit by 64-bit division instruction, find a way
// to use this? Probably require separate assembly language file.

// (quotient, remainder) = dividend / divisor
// divisor must be normalized - high word is non-zero
// all numbers presented in little-endian (q[0] is low word, etc)
// quotient needs m-n+1 words, remainder needs n words

// TBD - bump this to 32-bit if we have a 64-bit build (where we have
// access to wider division instructions - we do, right?). Could change
// this to a template on T=uint16_t or uint32_t.

bool LongDivide(
    uint16_t* q /*quotient*/, uint16_t* r /*remainder*/,
    uint16_t* u /*dividend*/, uint16_t* v /*divisor*/,
    int m /*dividend_size*/, int n /*divisor_size*/)
{
    const uint32_t b = 65536;
    const uint32_t WORD_MASK = b - 1;
    uint16_t un[8]; // normalized form of dividend (up to 128-bit)
    uint16_t vn[8]; // normalized form of divisor (up to 128-bit)
    uint32_t qhat; // estimated quotient digit
    uint32_t rhat; // a remainder
    uint32_t p; // a product
    //int s, i, j, t, k;

    memset(un, 0, sizeof(un)); // only for debugging
    memset(vn, 0, sizeof(vn)); // only for debugging

    // Sanity checks
    if (m < n || n <= 0 || v[n-1] == 0
        || n > sizeof(vn)/sizeof(vn[0])
        || m+1 > sizeof(un)/sizeof(un[0]))
        return false; // invalid parameters

    // Do we have a single-digit divisor? If so, we can do a very simplified
    // version of long division
    if (n == 1)
    {
        int k = 0;
        for (int j = m - 1; j >= 0; j--)
        {
            q[j] = uint16_t((k*b + u[j]) / v[0]);
            k    = (k*b + u[j]) - q[j] * v[0];
        }
        if (r != nullptr)
            r[0] = uint16_t(k);
        return true;
    }

    // Normalize by shifting v left just enough so that its high-order
    // bit is on, and shift u left the same amount. We may have to append
    // a high-order digit on the dividend; we do that unconditionally.
    int s = __lzcnt16(v[n-1]); // note this is a 16-bit instruction

    for (int i = n-1; i > 0; i--)
        vn[i] = (v[i] << s) | (v[i-1] >> (16-s));
    vn[0] = v[0] << s;

    un[m] = u[m-1] >> (16-s);
    for (int i = m-1; i > 0; i--)
        un[i] = (u[i] << s) | (u[i-1] >> (16-s));
    un[0] = u[0] << s;

    // Main loop
    for (int j = m-n; j >= 0; j--)
    {
        // Compute estimate qhat of q[j]
        qhat = (un[j+n]*b + un[j+n-1]) / vn[n-1];
        rhat = (un[j+n]*b + un[j+n-1]) - qhat*vn[n-1];

        // The estimate qhat, after the following steps, is either
        // exact or too high by 1, so this will repeat at most once.
        again:
        if (qhat >= b || qhat*vn[n-2] > b*rhat + un[j+n-2])
        {
            qhat = qhat - 1;
            rhat = rhat + vn[n-1];
            if (rhat < b)
                goto again;
        }

        // Multiply and subtract
        int k = 0;
        int t = 0;
        for (int i = 0; i < n; i++)
        {
            p = qhat * vn[i]; // needs to be 2x bits of input
            t = un[i+j] - k - (p & WORD_MASK); // needs to be 2x bits of input
            un[i+j] = uint16_t(t);
            k = (p >> 16) - (t >> 16);
        }
        t = un[j+n] - k; // this is why t needs to be signed
        un[j+n] = uint16_t(t);

        // Store quotient digit. If we subtracted too much, add back
        q[j] = uint16_t(qhat);
        if (t < 0)
        {
            q[j] = q[j] - 1;
            k = 0;
            for (int i = 0; i < n; i++)
            {
                t = un[i+j] + vn[i] + k;
                un[i+j] = uint16_t(t);
                k = t >> 16;
            }
            un[j+n] = uint16_t(un[j+n] + k);
        }
    }

    // If the caller wants the remainder, unnormalize it first
    if (r != nullptr)
    {
        for (int i = 0; i < n; i++)
            r[i] = (un[i] >> s) | (un[i+1] << (16-s));
    }

    return true;
}

// =========================================================================

#include "Bignum.h"

// Note that Bignums don't have overflows, they just get bigger. And if we did floating-point
// bignums, then there would be no underflow either.

Bignum::Bignum(int v)
{
    oplocal_[0] = v;
    size_ = 1;
}

Bignum::Bignum(long long v)
{
    oplocal_[0] = v & 0xFFFFFFFFL;
    oplocal_[1] = v >> 32;
    size_ = 2;
}

// for now, assume base 10 positive number
// TBD growing data past 24 bytes
// TBD multiply and add at the same time - mul_add_1? could carry more than 1 digit?
Bignum::Bignum(char const* p)
{
    size_ = 1;
    oplocal_[0] = 0;

    for (; *p; p++)
    {
        int digit = *p - '0';
        mul_1(10);
        #if 0
        int carry = 0;
        for (int i = 0; i < size_; i++)
        {
            long long r = carry + oplocal_[i] * 10LL;
            if (r >= 0x1'0000'0000L)
            {
                carry = r >> 32;
                if (i+1 == size_)
                {
                    size_ += 1;
                    oplocal_[i+1] = 0;
                }
            }
            oplocal_[i] = (uint32_t) r;
        }
        oplocal_[0] += digit; // grr this can overflow too!
        #endif
        add_1(digit);
    }
}

// Add a single place digit to a bignum
void Bignum::add_1(uint32_t digit)
{
    long long carry = digit;
    for (int i = 0; i < size_; i++)
    {
        long long r = carry + oplocal_[i];
        oplocal_[i] = (uint32_t) r;
        int hi = r >> 32;
        if (hi == 0)
            break;

        carry = hi;
        if (i+1 == size_)
        {
            size_ += 1;
            oplocal_[i+1] = 0;
        }
    }
}

// Multiply a bignum by a single place digit
void Bignum::mul_1(uint32_t plier)
{
    int carry = 0;
    for (int i = 0; i < size_; i++)
    {
        long long r = carry + oplocal_[i] * (long long) plier;
        int hi = r >> 32;
        if (hi != 0)
        {
            carry = hi;
            if (i+1 == size_)
            {
                size_ += 1;
                oplocal_[i+1] = 0;
            }
        }
        oplocal_[i] = (uint32_t) r;
    }
}

// consider operator"" _h, _d, _b etc.

Bignum::operator int() const
{
    return oplocal_[0];
}

Bignum::operator long long() const
{
    return ((long long) oplocal_[1] << 32) | oplocal_[0];
}

#include <cassert>

void TestBignum()
{
    Bignum b1("0");
    assert(b1.len() == 1); assert(b1[0] == 0);

    Bignum b2("100");
    assert(b2.len() == 1); assert(b2[0] == 100);

    Bignum b3("65536");
    assert(b3.len() == 1); assert(b3[0] == 65536);

    Bignum b4("4294967295");
    assert(b4.len() == 1); assert(b4[0] == 0xFFFFFFFFL);

    Bignum b4b("4294967296");
    assert(b4b.len() == 2); assert(b4b[0] == 0); assert(b4b[1] == 1);

    Bignum b5("102030405060708090");
    assert(b5.len() == 2); assert(b5[0] == 0xE1EEC6FA); assert(b5[1] == 0x16A7C1C);

    b5[2] = b5[1] + 1; // testing operator[] assign
    assert(b5[1] + 1 == b5[2]);
}
