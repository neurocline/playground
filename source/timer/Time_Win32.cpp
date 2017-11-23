// Time_Win32.cpp

#ifdef _WIN32
#include <Windows.h>
#include <intrin.h>

namespace
{
LARGE_INTEGER intNano;
LARGE_INTEGER fracNano;
}

//==============================================================================================
// Return nanoneconds using the fastest reliable timebase

uint64_t FastNanos()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    // base-2^32 multiply
    //   DE.00
    // *  A.BC

    // c0w0 is less than 1 bit of precision. We truncate by not computing it, where
    // we should have rounded up. This saves time and in reality produces the same
    // results.
    // uint64_t c0w0 = (uint64_t) counter.LowPart * fracNano.LowPart;
    uint64_t c0w1 = (uint64_t) counter.LowPart * fracNano.HighPart;
    uint64_t c0w2 = (uint64_t) counter.LowPart * intNano.LowPart;
    uint64_t c1w0 = (uint64_t) counter.HighPart * fracNano.LowPart;
    uint64_t c1w1 = (uint64_t) counter.HighPart * fracNano.HighPart;
    uint64_t c1w2 = (uint64_t) counter.HighPart * intNano.LowPart;

    uint64_t v = (c0w1 >> 32) + c0w2 + (c1w0 >> 32) + c1w1 + (c1w2 << 32)
               + (((c0w1 & 0xFFFFFFFF) + (c1w0 & 0xFFFFFFFF) + 0x80000000) >> 32);
    return v;
}

// On first use, initialize timebase

void InitFastNanos()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    uint16_t dividend[6];
    uint16_t divisor[4];

#if 0
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
#endif
}

#endif // defined(_WIN32)
