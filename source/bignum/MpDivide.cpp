// ======================================================================================
// MpDivide.cpp
//
// Implement Knuth multiprecision divide per Warren. This is a standalone file that
// can be put into any project.
// ======================================================================================

#include <cstdint>

// --------------------------------------------------------------------------------------

// Count leading zeros primitive. On Intel, this is the lzcnt or lzcnt32 instruction.
// At worst, we use a handwritten loop.

#if defined(_MSC_VER)
#include <intrin.h>
// this gives us __lzcnt16 and __lzcnt

#elif defined(__clang__) && __has_include(<x86intrin.h>)
#include <x86intrin.h>
#define __lzcnt(X) __lzcnt32(X)

#elif defined(__clang__)
static __inline__ unsigned short
__lzcnt16(unsigned short __X)
{
    return __X ? __builtin_clzs(__X) : 16;
}
static __inline__ unsigned int
__lzcnt(unsigned int __X)
{
    return __X ? __builtin_clz(__X) : 32;
}

#else
// straight C implementation - leading zeros through binary search
static unsigned short
__lzcnt16(unsigned short x)
{
    unsigned short y;
    unsigned short n = 16;
    for (unsigned short c = 8; c != 0; c >>= 1)
    {
        y = x >> c;
        if (y != 0)
        {
            n = n - c;
            x = y;
        }
        c = c >> 1;
    }
    return n - x;
}

static unsigned int
__lzcnt(unsigned int x)
{
    unsigned int y;
    unsigned int n = 32;
    for (unsigned int c = 16; c != 0; c >>= 1)
    {
        y = x >> c;
        if (y != 0)
        {
            n = n - c;
            x = y;
        }
        c = c >> 1;
    }
    return n - x;
}
#endif

// --------------------------------------------------------------------------------------

template <typename WORD>
struct ContainsType;

template<>
struct ContainsType<uint16_t>
{
    using type = uint32_t;
    static constexpr int shift = 16;
    static constexpr type base = 1 << 16;
    static constexpr type mask = base - 1;

    static int LeadingZeros(uint16_t v) { return __lzcnt16(v); }
};

template<>
struct ContainsType<uint32_t>
{
    using type = uint64_t;
    static constexpr int shift = 32;
    static constexpr type base = 1LL << 32;
    static constexpr type mask = base - 1;

    static int LeadingZeros(uint32_t v) { return __lzcnt(v); }
};

// --------------------------------------------------------------------------------------

template<typename WORD>
bool MultiwordDivide(
    WORD* quotient, WORD* remainder,
    const WORD* dividend, const WORD* divisor,
    int dividendSize, int divisorSize, WORD* scratch)
{
    using mathType = typename ContainsType<WORD>::type;
    const mathType b = ContainsType<WORD>::base;
    const mathType WORD_MASK = ContainsType<WORD>::mask;

    // Sanity checks
    if (dividendSize < divisorSize || divisorSize == 0)
        return false;

    WORD* q = quotient;
    WORD* u = (WORD *) dividend;
    WORD* v = (WORD *) divisor;

    // Special-case for single "digit" divisor (one WORD).
    // We can just work our way directly through the dividend, one WORD
    // at a time. This isn't just an optimization, the mainline code
    // expects divisors to have at least two digits.
    if (divisorSize == 1)
    {
        mathType r = 0;  // this is our remainder from the previous WORD
        for (int j = dividendSize - 1; j >= 0; j--)
        {
            mathType partial = r * b + u[j];
            q[j] = WORD(partial / v[0]);
            r = partial - q[j] * v[0];
        }
        if (remainder != nullptr)
            remainder[0] = WORD(r);
        return true;
    }

    // Set up temps for holding normalized divisor and dividend
    // TBD allocate if we need more space
    #if 0
    const int unSize = 8;
    const int vnSize = 8;
    WORD un[unSize];
    WORD vn[vnSize];
    // memset(un, 0, sizeof(un));
    // memset(vn, 0, sizeof(vn));

    if (divisorSize > vnSize || dividendSize+1 > unSize)
        return false;
    #else

    int n = divisorSize;
    int m = dividendSize;

    WORD iscratch[16];
    WORD* un;
    WORD* vn;
    if (scratch == nullptr && n+m+1 <= 16)
        scratch = iscratch;
    else if (scratch == nullptr)
        return false; // fail, need more storage (TBD make this allocate)

    vn = scratch;
    un = vn + n;
    #endif

    // Normalize divisor so that its high-order bit is set to 1.
    // Note uint16_t >> 16 is not undefined, but uint32_t >> 32 is undefined,
    // becasuse C promotes to int or uint when doing math. So we just
    // check and if we don't need to bitshift, we just copy
    int s = ContainsType<WORD>::LeadingZeros(v[n-1]);
    static constexpr int shift = ContainsType<WORD>::shift;
    if (s == 0)
    {
        for (int i = 0; i < n; i++) vn[i] = v[i];
        for (int i = 0; i < m; i++) un[i] = u[i];
        un[m] = 0;
    }
    else
    {

        for (int i = n-1; i > 0; i--)
            vn[i] = (v[i] << s) | (v[i-1] >> (shift-s));
        vn[0] = v[0] << s;

        // Shift dividend left by the same amount; this may mean appending a high-order
        // digit on the dividend, so we just always add it even if it is zero.
        un[m] = u[m-1] >> (shift-s);
        for (int i = m-1; i > 0; i--)
            un[i] = (u[i] << s) | (u[i-1] >> (shift-s));
        un[0] = u[0] << s;
    }

    // Main loop
    for (int j = m-n; j >= 0; j--)
    {
        // Compute estimate qhat of q[j]
        mathType qhat = (un[j+n]*b + un[j+n-1]) / vn[n-1];
        mathType rhat = (un[j+n]*b + un[j+n-1]) - qhat*vn[n-1];

        // The estimate qhat, after the following steps, is either
        // exact or too high by 1, so this will repeat at most once.
        #if 0
        again:
        if (qhat >= b || qhat*vn[n-2] > b*rhat + un[j+n-2])
        {
            qhat = qhat - 1;
            rhat = rhat + vn[n-1];
            if (rhat < b)
                goto again;
        }
        #else
        do
        {
            if (qhat < b && qhat*vn[n-2] <= b*rhat + un[j+n-2])
                break;
            qhat = qhat - 1;
            rhat = rhat + vn[n-1];
        } while (rhat < b);
        #endif

        // Multiply and subtract
        int64_t k = 0; // fix me
        int64_t t = 0;
        for (int i = 0; i < n; i++)
        {
            mathType p = qhat * vn[i]; // needs to be 2x bits of input
            t = un[i+j] - k - (p & WORD_MASK); // needs to be 2x bits of input
            un[i+j] = WORD(t);
            k = (p >> shift) - (t >> shift);
        }
        t = un[j+n] - k; // this is why t needs to be signed
        un[j+n] = WORD(t);

        // Store quotient digit. If we subtracted too much, add back
        q[j] = WORD(qhat);
        if (t < 0)
        {
            q[j] = q[j] - 1;
            k = 0;
            for (int i = 0; i < n; i++)
            {
                t = un[i+j] + vn[i] + k;
                un[i+j] = WORD(t);
                k = t >> shift;
            }
            un[j+n] = WORD(un[j+n] + k);
        }
    }

    // If the caller wants the remainder, unnormalize it first
    if (remainder != nullptr)
    {
        for (int i = 0; i < n; i++)
            remainder[i] = (un[i] >> s) | (un[i+1] << (shift-s));
    }

    return true;
}

// force instantiation of uint32_t version
template
bool MultiwordDivide<uint32_t>(
    uint32_t* quotient, uint32_t* remainder,
    const uint32_t* dividend, const uint32_t* divisor,
    int dividendSize, int divisorSize, uint32_t* scratch);
