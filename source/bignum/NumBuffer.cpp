// ======================================================================================
// NumBuffer.cpp
// - buffer class for arbitrary precision numbers
// ======================================================================================

#include "Num.h"

#include <cassert>

// ======================================================================================
// Basic constructors
// - the empty constructor makes a small NumBuffer that's zero-length
// - copy constructors make the minimal-sized NumBuffer; this may shrink data
// - move constructors make the rhs NumBuffer contain zero
// ======================================================================================

// Default constructor - create empty NumBuffer (zero-length small Num)
NumBuffer::NumBuffer()
{
    // Initialize metadata (compiler should turn this into a single instruction)
    nonlocal = 0;
    sign = 0;
    len = 0;
}

// Destructor - free any NumBuffer-related data
NumBuffer::~NumBuffer() noexcept
{
    // If there was allocated data, free it and zero out pointer (will force crash
    // if object referenced after destruction)
    if (nonlocal)
        delete[] big.digits;

    nonlocal = 1;
    big.digits = nullptr;
}

// Copy constructor
// - A newly constructed object is the minimum size that it needs to be in order
//   to hold the rhs.
NumBuffer::NumBuffer(const NumBuffer& rhs) noexcept
{
    // Copy the prefix: local + size + len
    nonlocal = rhs.nonlocal;
    sign = rhs.sign;
    len = rhs.len;

    // If the rhs is a small NumBuffer, just copy the whole thing, since a new NumBuffer is
    // a small NumBuffer (no allocations needed)
    if (!rhs.nonlocal)
        copy_digits(buf, rhs.buf, len);

    // If the rhs NumBuffer has a big buffer but a small length, then we "shrink" it.
    // We do this because our canonical way to shrink buffers is to copy to a new
    // object.
    else if (rhs.len <= smallbufsize)
    {
        nonlocal = 0;
        copy_digits(buf, rhs.big.digits, rhs.len);
    }

    // Otherwise, when copying big data, create a new buffer to copy into. Again, we
    // shrink to the actual size on the rhs, not the rhs.bufsize
    else
    {
        big.bufsize = len;
        big.digits = new uint32_t[big.bufsize];
        copy_digits(big.digits, rhs.big.digits, len);
    }
}

// Move constructor
// - transfer data to lhs as-is
// - we leave the rhs in a broken state - it's not an empty NumBuffer, it's an
//   invalid one.
NumBuffer::NumBuffer(NumBuffer&& rhs) noexcept
{
    // move data
    move_(rhs);
}

// ======================================================================================
// Basic copy assignment operators
// ======================================================================================

// Copy assignment operator
// - copy assignment doesn't shrink the dest buf
// - copy assignment only grows the dest buf as needed
NumBuffer& NumBuffer::operator=(const NumBuffer& rhs) noexcept
{
    if (this == &rhs)
        return *this; // do we REALLY need to be paranoid like this? I mean, really...

    // This differs from construction in that we will keep an overlarge buffer;
    // this is one of the two cases where an allocated buffer can hold data less
    // than smallbufsize in length (the other is from math operators that produce
    // a smaller result than the lhs operand)

    // See if the new number will fit in the current space.
    int bufsize = nonlocal ? big.bufsize : smallbufsize;
    if (rhs.len > bufsize)
    {
        // If we have nonlocal data, free it; it's not big enough, so we will
        // allocate more.
        if (nonlocal)
        {
            delete[] big.digits;
            nonlocal = 0; // temporarily a small Num
        }

        // If the data won't fit into a local buffer, allocate a new big one.
        if (rhs.len > smallbufsize)
        {
            nonlocal = 1;
            big.bufsize = rhs.len;
            big.digits = new uint32_t[big.bufsize];
        }
    }

    // Copy the digits
    copy_digits(digits(), rhs.cdigits(), rhs.len);

    // Copy length and sign
    sign = rhs.sign;
    len = rhs.len;

    return *this;
}

// Move assignment operator
// - move rhs to lhs and leave lhs in an invalid state. This differs from the
//   copy assignment operator in that we don't try to use existing buffers. We
//   could actually share code with the move constructor.
NumBuffer& NumBuffer::operator=(NumBuffer&& rhs) noexcept
{
    if (this == &rhs)
        return *this; // do we REALLY need to be paranoid like this? I mean, really...

    // Destroy any existing buffer
    if (nonlocal)
        delete[] big.digits;

    // move data
    move_(rhs);

    return *this;
}

// Implementation of move constructor and move assignment operator
// - note that a faster move could just memcpy the whole object and then fiddle
//   with the rhs
void NumBuffer::move_(NumBuffer& rhs)
{
    // Copy metadata
    nonlocal = rhs.nonlocal;
    sign = rhs.sign;
    len = rhs.len;

    // If nonlocal, then we just need to move the pointer and buffer size.
    if (nonlocal)
    {
        big.digits = rhs.big.digits;
        big.bufsize = rhs.big.bufsize;
    }

    // If local, copy data. 
    else
        copy_digits(buf, rhs.buf, len);

    // turn the rhs into a broken object that will crash the program if used
    // (harsh but important).
    rhs.nonlocal = 1;
    rhs.big.digits = nullptr; // invalid object now
}

// ======================================================================================
// Buffer management
// ======================================================================================

// Reserve space (used for a NumBuffer we know is going to get large).
// This will never reduce the NumBuffer below the size of the current buffer. The way
// to shrink an allocation is to copy-construct a new NumBuffer from an existing one.
uint32_t* NumBuffer::reserve(int size)
{
    // If we are already at least this size, ignore
    if (size <= smallbufsize || (nonlocal && size <= big.bufsize))
        return digits();

    // Allocate new buffer
    uint32_t* newdigits = new uint32_t[size];

    // Copy existing data into it
    copy_digits(newdigits, digits(), len);

    // If there is an existing buffer, release it
    if (nonlocal)
        delete[] big.digits;

    nonlocal = 1;
    big.bufsize = size;
    big.digits = newdigits;

    return newdigits;
}

// Resize NumBuffer. This can shrink (and throw away data), or it can grow
// (and leave new uninitialized data). We trust the caller to know what they are doing.
uint32_t* NumBuffer::resize(int size)
{
    // shrinking is trivial, just reduce len (leave a big NumBuffer as a big NumBuffer)
    if (size < len)
    {
        len = size;
        if (len == 0)
            sign = 0; // zero is always a positive number
    }

    // Growing is a bit more complicated, because we may have to allocate a new buffer.
    // We don't just set capacity to the new size, because some work patterns involve
    // resizing as the algorithm progresses. We increase by 1.5x each time.
    else
    {
        // If we need a new buffer, make one by growing at a 1.5x rate
        if (size > capacity())
        {
            int newsize = capacity(); 
            while (size > newsize)
                newsize = newsize * 3 / 2;

            // Copy existing information and replace with our upsized buffer
            uint32_t* newdigits = new uint32_t[newsize];
            copy_digits(newdigits, digits(), len);

            if (nonlocal)
                delete[] big.digits;

            big.digits = newdigits;
            big.bufsize = newsize;
            nonlocal = 1;
        }        

        len = size;
    }

    return digits();
}
