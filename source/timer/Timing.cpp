// Timing.cpp

#include "Timing.h"

#if 0

#include <cassert>

#ifdef _MSC_VER
#include <Windows.h>
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

bool LongDivide(
    uint16_t* q /*quotient*/, uint16_t* r /*remainder*/,
    uint16_t* u /*dividend*/, uint16_t* v /*divisor*/,
    int m /*dividend_size*/, int n /*divisor_size*/);

LARGE_INTEGER intNano;
LARGE_INTEGER fracNano;

void InitFastNanos()
{
    // TBD actual measurement and long-precision division
    intNano.QuadPart = 0xFFLL;
    fracNano.QuadPart = 0x7D3B3CCDFC1006E2LL;

    // Now use LongDivide and make sure we have the same thing
    uint16_t divisor[4];
    uint16_t dividend[8];

    uint32_t div = 1'000'000'000L;
    memset(dividend, 0, sizeof(dividend));
    dividend[4] = div & 0xFFFF; div >>= 16;
    dividend[5] = div & 0xFFFF; div >>= 16;
    dividend[6] = div & 0xFFFF; div >>= 16;
    dividend[7] = div & 0xFFFF; div >>= 16;

    div = 3914060;
    divisor[0] = div & 0xFFFF; div >>= 16;
    divisor[1] = div & 0xFFFF; div >>= 16;
    divisor[2] = div & 0xFFFF; div >>= 16;
    divisor[3] = div & 0xFFFF; div >>= 16;

    uint16_t quotient[5];
    uint16_t remainder[4];
    bool ok = LongDivide(quotient, remainder, dividend, divisor, 6, 2);
    assert(ok);
    assert(quotient[0] == 0x06E2);
    assert(quotient[1] == 0xFC10);
}

void TestLongDivide()
{
    uint16_t dividend[8];
    uint16_t divisor[8];
    uint16_t quotient[8];
    uint16_t remainder[8];

    memset(dividend, 0, sizeof(dividend));
    memset(divisor, 0, sizeof(divisor));
    memset(quotient, 0, sizeof(quotient));
    memset(remainder, 0, sizeof(remainder));

    dividend[4] = 1;
    divisor[3] = 0x0100;

    bool ok = LongDivide(quotient, remainder, dividend, divisor, 5, 4);
    assert(ok);
    assert(quotient[0] == 256);
}

uint64_t FastNanos()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    // c0w0 is less than 1 bit of precision. We lose a fraction of a bit by not computing it
    uint64_t c0w1 = (uint64_t) counter.LowPart * fracNano.HighPart;
    uint64_t c0w2 = (uint64_t) counter.LowPart * intNano.LowPart;
    uint64_t c1w0 = (uint64_t) counter.HighPart * fracNano.LowPart;
    uint64_t c1w1 = (uint64_t) counter.HighPart * fracNano.HighPart;
    uint64_t c1w2 = (uint64_t) counter.HighPart * intNano.LowPart;

    int64_t v = (c0w1 >> 32) + c0w2 + (c1w0 >> 32) + c1w1 + (c1w2 << 32)
                    + (((c0w1 & 0xFFFFFFFF) + (c1w0 & 0xFFFFFFFF) + 0x80000000) >> 32);
    return v;
}

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
        if (qhat >= b || qhat*vn[n-2] > b*rhat + un[j+n-1])
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

//*************************************************************************************************
//*************************************************************************************************

#if 0

// this works, but is terrible

template<typename T>
auto perf_counter() -> T
{
    T value = 0;
    return value;
}

template<>
auto perf_counter() -> double
{
    return 1.0;
}

template<>
auto perf_counter() -> int64_t
{
    return 2;
}

#include <iostream>

void p(double v)
{
    uint64_t i = *(uint64_t*)&v;
    std::cout << v << "=" << i << "   ";
}

void pd(double v)
{
    uint64_t i = *(uint64_t*)&v;
    int64_t s = (i >> 63) & 1;
    int64_t m = (i >> 52) & 0x7FF;
    int64_t f = i & 0xFFFFFFFFFFFFFL;
    std::cout << v << "=" << s << "," << m << "," << f << "; ";
}

void test_double_range()
{
    double billion = 1'000'000'000.0;
    for (double num = 1.0; num < 10*billion; num *= 2)
    {
        double next = num;
        uint64_t* pnext = (uint64_t*)&next;
        while (next - num == 0.0)
            *pnext += 1;
        double y = num / (86400 * 365.25);
        std::cout << y << " ";
        std::cout << (num / 86400) << ": ";
        pd(num);
        pd(next);
        std::cout << next - num;
        std::cout << std::endl;
    }
}

void test()
{
    test_double_range();

    auto start_double = perf_counter<double>();
    auto end_int64 = perf_counter<int64_t>();

    double day = 86400.0;
    double year = day * 365.25;
    double billion = 1'000'000'000.0;

    for (int y = 0; y < 100; y++)
    {
        std::cout << y << ": ";
        double t1 = y*year + 0.001;
        uint64_t t1m = *(uint64_t*)&t1 & 0x000F'FFFF'FFFF'FFFFL;
        pd(t1);
        double t2 = t1 + 1'000'000.0/billion;
        uint64_t t2m = *(uint64_t*)&t2 & 0x000F'FFFF'FFFF'FFFFL;
        pd(t2);
        double d = t2 - t1;
        pd(d);
        // std::cout << (t2m - t1m);
        double nanos = d * billion;
        std::cout << (int64_t) nanos;
        std::cout << std::endl;
    }
}
#endif
#endif

