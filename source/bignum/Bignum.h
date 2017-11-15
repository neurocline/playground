// Bignum.h

#pragma once

#include <stdint.h>
#include <vector>

// (quotient, remainder) = dividend / divisor
// divisor must be normalized so that high word is non-zero
// all numbers presented in little-endian (q[0] is low word, etc)
// quotient needs m-n+1 words, remainder needs n words

bool LongDivide(
    uint16_t* q /*quotient*/, uint16_t* r /*remainder*/,
    uint16_t* u /*dividend*/, uint16_t* v /*divisor*/,
    int m /*dividend_size*/, int n /*divisor_size*/);

// =========================================================================

class Bignum
{
    enum { LOCALSIZE = 7 };
public:
    Bignum() : size_(0) { oplocal_[0] = 0; }
    ~Bignum() { if (size_ > LOCALSIZE) delete extended; }

    Bignum(int v);
    Bignum(long long v);
    Bignum::Bignum(char const* p);

    operator int() const;
    operator long long() const;

    int len() { return size_; }
    uint32_t operator [](int i) { return oplocal_[i]; }

    void mul_1(uint32_t plier);
    void add_1(uint32_t digit);

private:
    // A Bignum takes up 32 bytes of stack storage. Larger values are
    // allocated on the heap. Bignums aren't cheap, so don't worry about space.
    // TBD something other than vector
    int size_;
    union
    {
        uint32_t oplocal_[LOCALSIZE];
        std::vector<uint32_t>* extended;
    };
};

void TestBignum();

// =========================================================================

#include <intrin.h>

template <typename WORD>
class ContainsType;

template<>
struct ContainsType<uint16_t>
{
    using type = uint32_t;
    static constexpr int shift = 16;
    static constexpr type base = 1 << 16;
    static constexpr type mask = base - 1;

    static constexpr int LeadingZeros(uint16_t v) { return __lzcnt16(v); }
};

template<>
struct ContainsType<uint32_t>
{
    using type = uint64_t;
    static constexpr int shift = 32;
    static constexpr type base = 1LL << 32;
    static constexpr type mask = base - 1;

    static constexpr int LeadingZeros(uint32_t v) { return __lzcnt(v); }
};

template<typename WORD>
bool MultiwordDivide(
    WORD* quotient, WORD* remainder,
    WORD* dividend, WORD* divisor,
    int dividendSize, int divisorSize)
{
    using mathType = ContainsType<WORD>::type;
    const mathType b = ContainsType<WORD>::base;
    const mathType WORD_MASK = ContainsType<WORD>::mask;
    
    // Sanity checks
    if (dividendSize < divisorSize || divisorSize == 0)
        return false;

    WORD* q = quotient;
    WORD* u = dividend;
    WORD* v = divisor;

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
    const int unSize = 8;
    const int vnSize = 8;
    WORD un[unSize];
    WORD vn[vnSize];
    // memset(un, 0, sizeof(un));
    // memset(vn, 0, sizeof(vn));

    if (divisorSize > vnSize || dividendSize+1 > unSize)
        return false;

    int n = divisorSize;
    int m = dividendSize;

    // Normalize divisor so that its high-order bit is set to 1.
    int s = ContainsType<WORD>::LeadingZeros(v[n-1]);
    static constexpr int shift = ContainsType<WORD>::shift;

    for (int i = n-1; i > 0; i--)
        vn[i] = (v[i] << s) | (v[i-1] >> (shift-s));
    vn[0] = v[0] << s;

    // Shift dividend left by the same amount; this may mean appending a high-order
    // digit on the dividend, so we just always add it even if it is zero.
    un[m] = u[m-1] >> (shift-s);
    for (int i = m-1; i > 0; i--)
        un[i] = (u[i] << s) | (u[i-1] >> (shift-s));
    un[0] = u[0] << s;

    // Main loop
    for (int j = m-n; j >= 0; j--)
    {
        // Compute estimate qhat of q[j]
        mathType qhat = (un[j+n]*b + un[j+n-1]) / vn[n-1];
        mathType rhat = (un[j+n]*b + un[j+n-1]) - qhat*vn[n-1];

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
