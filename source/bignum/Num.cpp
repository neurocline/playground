// ======================================================================================
// Num.cpp
// - arbitrary-precision numbers with basic operator support
//
// Basic Num support - constructors/destructors, and serialization
// ======================================================================================

#include "Num.h"

#include <cassert>
#include <cstring>
#include <cstdint>

// ======================================================================================
// Basic constructors
// - the empty constructor makes a small Num that's zero-length
// - copy constructors make the minimal-sized Num; this may shrink data
// ======================================================================================

// Default constructor - create empty Num (zero-length small Num)
Num::Num()
{
    #if 1
    // There are two approaches here. The first is the minimalist-time approach, which
    // only initializes the bare minumum. This matches the design of C, in which stack
    // variables aren't initialized to a specific value, just to a legal value.
    data.local = 1;
    small.sign = 0;
    small.len = 0;

    #else
    // The other approach is to completely initialize it. This is probably less code,
    // but likely runs a little slower.
    memset(&small, 0, sizeof(small));
    data.local = 1;
    #endif

    // All of this points out something interesting with our design. Maybe we should
    // make the first bit be a "nonlocal" bit so it can be initialized to 0. And really,
    // none of this matters, because we expect most Nums to be large and thus need
    // allocation.
}

// Destructor - free any Num-related data
Num::~Num() noexcept
{
    // If there was allocated data, free it
    if (!data.local)
        delete[] big.buf;
}

// Copy constructor (only used on new unconstructed object)
Num::Num(const Num& rhs) noexcept
{
    // If small data on rhs, then no allocation needed on new object, just copy the
    // old one completely. This will on average copy too much, but we don't care,
    // trying to figure out how much to copy is probably slower.
    if (rhs.data.local)
        memcpy(&small, &rhs.small, sizeof(small));

    // If the rhs Num would fit in a small object, then we "shrink" it. We do this
    // because the canonical way to shrink buffers is to copy to a new object.
    else if (rhs.data.len <= smallbufsize)
    {
        data.local = 1;
        small.sign = rhs.big.sign;
        small.len = rhs.data.len;
        memcpy(small.buf, rhs.big.buf, data.len*4);
    }

    // We have big data on rhs, so copy metadata and create a new buffer
    // (no copy-on-write buffer sharing! bad idea!).
    else
    {
        memcpy(&big, &rhs.big, sizeof(big));
        big.bufsize = big.len;
        big.buf = new uint32_t[big.bufsize];
        memcpy(big.buf, rhs.big.buf, big.len*4);
    }
}

// ======================================================================================
// Basic copy assignment operators
// - copy assignment only grows the dest buf as needed
// - copy assignment doesn't shrink the dest buf
// ======================================================================================

// Copy assignment operator
Num& Num::operator=(const Num& rhs) noexcept
{
    if (this == &rhs)
        return *this; // do we REALLY need to be paranoid like this? I mean, really...

    // Copying from a small Num
    if (rhs.data.local)
    {
        // If we are copying a small Num to a small one, then just blockmove the data
        if (data.local)
            memcpy(&small, &rhs.small, sizeof(Num::small));

        // If we are copying a small Num to a big one, don't downgrade this Num, just
        // copy data. We know we already have an allocated buffer big enough.
        else
        {
            big.sign = rhs.small.sign;
            big.len = rhs.small.len;
            memcpy(big.buf, rhs.small.buf, rhs.small.len*4);
        }
    }

    // Copying from a large Num
    else
    {
        // If the rhs won't fit, free any existing lhs data and make this look like a small Num
        if (!data.local && rhs.big.len > big.bufsize)
        {
            delete[] big.buf;
            data.local = 1;
        }

        // If we have no buffer, allocate one. Use the shorter size from the rhs.
        // TBD we could downgrade here if rhs.len <= smallbufsize
        if (data.local)
        {
            data.local = 0;
            big.bufsize = rhs.big.len;
            big.buf = new uint32_t[big.bufsize];
        }

        // Copy data
        big.sign = rhs.big.sign;
        big.len = rhs.big.len;
        memcpy(big.buf, rhs.big.buf, big.len*4);
    }

    return *this;
}

// ======================================================================================
// Copy constructors that convert other types to Num
// ======================================================================================

// initialize a Num from a long long value. Pick the shortest Num
// that holds the value.
Num::Num(int v) noexcept
{
    data.local = 1;
    from_int64(v);
}

Num::Num(unsigned int uv) noexcept
{
    data.local = 1;
    from_uint64(uv);
}

Num::Num(long long v) noexcept
{
    data.local = 1;
    from_int64(v);
}

Num::Num(unsigned long long uv) noexcept
{
    data.local = 1;
    from_uint64(uv);
}

void Num::from_int64(long long v)
{
    unsigned long long uv = v;
    if (v < 0)
        uv = (~uv + 1);

    from_uint64(uv);

    if (v < 0)
        small.sign = -1;
}

// TBD - this breaks the "we won't deallocate reserved buffers" statement. Should we
// keep that promise, even though it makes for longer code in this instance?
void Num::from_uint64(unsigned long long uv)
{
    data.sign = 0;
    data.len = 0;
    if (!data.local)
    {
        delete[] big.buf;
        data.local = 0;
    }

    int i = 0;
    for (; uv != 0; i++)
    {
        small.len += 1;
        small.buf[i] = (uv & 0xFFFFFFFFL);
        uv >>= 32;
    }

    #ifndef NDEBUG
    for (; i < smallbufsize; i++)
        small.buf[i] = 0;
    #endif
}

#if 0

// Copy constructor from char*
Num::Num(char const* p, int base)
{
    from_cstring(p, base);
}

#endif

// ======================================================================================
// Copy assigment operators that convert other types to Num
// ======================================================================================

Num& Num::operator=(int v) noexcept
{
    from_int64(v);
    return *this;
}

Num& Num::operator=(unsigned int uv) noexcept
{
    from_uint64(uv);
    return *this;
}

Num& Num::operator=(long long v) noexcept
{
    from_int64(v);
    return *this;
}

Num& Num::operator=(unsigned long long uv) noexcept
{
    from_uint64(uv);
    return *this;
}

#if 0
// Copy assignment from char* (only base 10)
Num& Num::operator=(char const* p)
{
}
#endif


// ======================================================================================
// Type conversions
// ======================================================================================

#if 0

Num::operator int() const
{
    return int(to_int64());
}

Num::operator unsigned int() const
{
    return uint32_t(to_uint64());
}

Num::operator long long() const
{
    return to_int64();
}

Num::operator unsigned long long() const
{
    return to_uint64();
}

// Convert a Num to an unsigned long long value (modulo 2^64)
uint64_t Num::to_uint64() const
{
    const uint32_t* buf = data.local ? small.buf : big.buf;
    uint64_t uv = 0;
    int len = small.local ? small.len : big.len;
    if (len > 2) len = 2;

    for (int i = len - 1; i >= 0; i--)
        uv = (uv << 32) | buf[i];

    return uv;
}

// Convert a Num to a long long value (modulo 2^63)
int64_t Num::to_int64() const
{
    uint64_t uv = to_uint64();
    uv = (uv << 1) >> 1; // lop off high-order bit

    long long v = data.sign ? -(long long)uv : (long long)uv;

    return v;
}

// Convert Num to zero-terminated string.
// Return size of buffer required
int Num::to_cstring(char* p, int buflen, int base)
{
    int i = 0;
    #define PUT(c) { if (i < buflen) p[i] = c; i++; }

    // Now convert the number by repeatedly dividing it by the base
    Num t{*this};
    int sign = data.sign;
    if (sign)
        PUT('-')
    data.sign = 0;

    Num B{base};
    Num quotient;
    Num remainder;

    do
    {
        uint32_t r = t.divmod(base, quotient);
        char ch = '0' + (char) r;
        if (r > 9) ch += ('A' - '9' + 1); // turn 10+ into 'A'+

        PUT(ch);
        t = quotient; // replace with pointer swap
        t.trim();
    } while (t.len() != 0);

    PUT(0)

    // now reverse the string
    int j = sign ? 1 : 0; // skip any leading '-' sign
    int k = i - 2; // leave zero-terminator in place
    while (j < k)
    {
        char c = p[j]; p[j] = p[k]; p[k] = c;
        j++;
        k--;
    }
    return i;
    #undef PUT
}

// Convert zero-terminated string to Num
// TBD error handling on input instead of turning garbage into garbage
bool Num::from_cstring(char const* p, int base)
{
    resize(0);
    
    for (; *p; p++)
    {
        char digit = *p - '0';
        if (base > 10 && digit >= 10)
            digit = (*p & 0x0F) + 9;

        operator*=(uint32_t(base));
        operator+=(uint32_t(digit));
    }
    return true;
}

#endif

// ======================================================================================
// Buffer management
// ======================================================================================

// Reserve space (used for a Num we know is going to get large)
// This will never reduce the Num buffer below the size of the current buffer. The way
// to shrink an allocation is to copy-construct a new number from an existing one,
// or copy-assign into a small Num.
void Num::reserve(int size)
{
    // If we are already at least this size, ignore
    if (size <= smallbufsize || !data.local && size <= big.bufsize)
        return;

    // Allocate new buffer
    uint32_t* newbuf = new uint32_t[size];

    // Copy existing data into it
    memcpy(newbuf, databuffer(), data.len*4);

    // If there is an existing buffer, release it
    if (!data.local)
        delete[] big.buf;

    data.local = 0;
    big.bufsize = size;
    big.buf = newbuf;
}

// Resize Num
uint32_t* Num::resize(int size)
{
    if (size == data.len)
        return databuffer();
    else if (size < data.len)
        return shrink(data.len - size);
    else
        return grow(size - data.len);
}

// Grow the Num length.
// Return a pointer to the Num buffer, since grow can reallocate data.
// This initializes new storage.
uint32_t* Num::grow(int amt)
{
    int newlen = data.len + amt;
    int32_t obufsize = capacity();
    uint32_t* obuf = databuffer();

    // If we have a small buf that isn't big enough, or we have a big buf that has no
    // more space in reserve, allocate a new buffer and copy the existing data into it
    if (newlen > obufsize)
    {
        int32_t newbufsize = obufsize;
        while (newbufsize < newlen)
            newbufsize *= 2;

        uint32_t* newbuf = new uint32_t[newbufsize];
        memcpy(newbuf, obuf, data.len*4);

        if (!data.local)
            delete[] obuf;

        data.local = 0;
        big.buf = newbuf;
        big.bufsize = newbufsize;
        obuf = newbuf;
    }

    // Optionally initialize the new part of the Num
    // (TBD get rid of the need for this?)
    memset(&obuf[data.len], 0, amt*sizeof(uint32_t));
    data.len = newlen;

    return obuf;
}

// Remove the most significant digit from Num (assumed to be zero)
// This will not get rid of any allocation or shrink a buffer
uint32_t* Num::shrink(int amt)
{
    data.len -= amt;

    if (data.len < 0)
        data.len = 0;
    if (data.len == 0)
        data.sign = 0; // zero is always a positive number

    return databuffer();
}

// Resize Num so that the highest digit is non-zero
void Num::trim()
{
    auto b = databuffer();
    int i = data.len;
    for (; i > 0; --i)
        if (b[i-1] != 0)
            break;

    if (i != data.len)
        shrink(data.len - i);
}

// ======================================================================================
// Comparison
// ======================================================================================

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
    auto ld = databuffer();
    auto rd = rhs.cdatabuffer();
    for (int i = data.len - 1; i >= 0; --i)
    {
        if (ld[i] != rd[i])
            return ld[i] > rd[i] ? 1 : -1;
    }

    // Numbers have the same magnitude
    return 0;
}

// Num <=> digit
// question - what does it mean to comparing a signed Num vs an unsigned digit?
// maybe this is actually nonsense.
int Num::magcmp(const uint32_t& digit)
{
    // Check edge cases first
    if (len() > 1)
        return 1;
    else if (len() == 0)
        return digit == 0 ? 0 : -1;

    auto ld = databuffer();
    return ld[0] > digit ? 1 : ld[0] < digit ? -1 : 0;
}

#if 0
// Read from underlying data array
uint32_t Num::operator[](int i) const
{
    return cdatabuffer()[i];
}

uint32_t& Num::operator [](int i)
{
    return databuffer()[i];
}

// TBD
// - use std::copy to copy arrays
// - add allocation for large Num values

#endif

// ======================================================================================
// TBD stuff
// ======================================================================================

// TBD add dynamic data sizing
// TBD Small Object Optimization

// For small object optimization, there are several sizes to consider
// 16 bytes: 1 bit small-object, 1 bit sign, 6 bits length, 15 bytes number
// 24 bytes: 1 bit small-object, 1 bit sign, 6 bits length, 23 bytes number
// 32 bytes 1 bit small-object, 1 bit sign, 6 bits length, 31 bytes number
//
// Note that a pointer is 8 bytes on 64-bit architecture. We probably want to think
// in units of pointer-size, so that means 16-bytes is the smallest possible.
// 16 bytes = 120 bits of number, 24 bytes = 184 bits of number, 32 bytes = 248 bits of number
// 2^120 = 1.32e36 (approximately the range of 32-bit float)
// 2^184 = 2.45e55
// 2^248 = 4.52e74
// 64-bit float has range 1e-308 to 1e308 but only 53 bits of precision
// 100! = 9.3e157

// --------------------------------------------------------------------------------------

// TBD - create a make_Num helper that creates a single allocation containing both the
// Num and its allocated data (just like make_shared creates a shared pointer and its
// owned data in a single allocation).
// Of course, for this to mean anything, it means we need to create a Num on the heap
// with a predefined reserve. It's hard to figure out what this API should look like
// and be at all consistent with the rest of the class.
// And the destructor for an object allocated this way is different, because there
// is no freeing of the underlying storage, and even worse, we can't resize it. So
// maybe this API is a dumb idea.
#if 0
Num* make_Num(int size)
{
    return nullptr;
}
#endif

