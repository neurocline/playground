Num - a big number library
==========================

Num's goals are twofold

- readable and correct code
- small runtime possible

Most arbitrary number libraries sprawl - they are huge, their code bases are hard
to understand, and their use takes effort.

Contrast this with Python, which has a bignum library at its core - you just do math
on numbers. Can we have the same simplicity in C++?

```
#include "Num.h"

int main(int argc, char** argv)
{
    std::cout << "100! = " << factorial(100) << std::endl;
}

Num factorial(Num i)
{
    return i < 2 ? 1 : i * factorial(i - 1);
}
```

The NumBuffer class
===================

A `Num` is a variable-length object. We use a small object optimization so that `Num` values
up to around 10^30 fit into the static data in the `Num` object instead of being allocated.

`NumBuffer` contains the logic for such a variable-length object with small object optimization
applied. This means, for one, that `Num` itself doesn't need any constructors, destructors,
copy assignment or move assignment operators, since `Num` is a class that holds an instance
of `NumBuffer` and the operators to treat it as a number.

The sketch for `NumBuffer` is this:

```
class NumBuffer
{
public:
    NumBuffer();

    // Big 5: destructor, copy constructor, copy assignment operator
    // (move constructor and move assignment operator are default generated)
    ~NumBuffer() noexcept;
    NumBuffer(const NumBuffer& other) noexcept;            // Copy constructor
    NumBuffer& operator=(const NumBuffer& other) noexcept; // Copy assignment operator
    NumBuffer(NumBuffer&& rhs) noexcept;                   // Move constructor
    NumBuffer& operator=(NumBuffer&& rhs) noexcept;        // Move assignment operator

    // Do the work of move construction/move assignment operator
    void move_(NumBuffer& rhs);

    //... data manipulation functions (see below) ...//

    // The size of a small NumBuffer in digits.
    // At the moment, we have sizeof(NumBuffer) == 32
    static constexpr int smallbufsize = 7;

    uint32_t nonlocal : 1; // set to 0 for small data optimization
    int32_t sign : 1; // 0 for positive, -1 for negative
    int32_t len : 30; // this is too big, but there's nothing else to use it for yet

    union
    {
        uint32_t buf[smallbufsize];

        struct
        {
            int32_t bufsize;
            uint32_t* digits; // want different name than buf to catch bugs
        } big;
    };
};
```

We have hand-tuned `sizeof(NumBuffer)` to be 32 bytes. This fits in a cache line, which is
important if you have arrays of `Num`.

The two natural sizes for `NumBuffer` (and thus `Num`) are 16 bytes and 32 bytes.

- 16 bytes: array of 7 uint16_t digits plus metadata
- 32 bytes: array of 7 uint32_t digits plus metadata

The two reasons to prefer the 32-byte variant is that it simplifies the handling of `len`,
and it gives sufficient range. Otherwise, a small object `Num` is only 75% bigger than that
of a `long long`, and that doesn't seem practical. Of course, in real applications, maybe we
will find that all `Num` values are very large, and even 32 bytes isn't enough. To that
end, we might templatize `NumBuffer` so that we can make those changes easily.

There are a small number of functions in `NumBuffer` to manipulate `NumBuffer` data.

```
    // Resize a NumBuffer - this sets length and will grow capacity as needed.
    // It returns the digits as a convenience, since the buffer may have moved.
    uint32_t* resize(int size);

    // Reserve space for a large NumBuffer. This can never ben used to shrink the size
    // of a NumBuffer. Returns a pointer to the (new) buffer as a convenience.
    uint32_t* reserve(int size);

    // Return the size of the value in the NumBuffer
    int length() { return len; }

    // Return the capacity of the NumBuffer - the allocated storage
    int capacity() { return nonlocal ? big.bufsize : smallbufsize; }

    // Return a pointer to the start of NumBuffer storage.
    // A NumBuffer is stored as an array of digits. The non-template version of
    // NumBuffer is hardcoded to uint32_t as the digit size.
    uint32_t* digits() { return nonlocal ? big.digits : buf; }
    const uint32_t* cdigits() const { return nonlocal ? big.digits : buf; }

    // Clear out part of a Num
    // TBD get rid of the need for this
    // void clear_digits(int b, int e) { memset(digits()+b, 0, (e - b)*sizeof(uint32_t)); }

    // Copy a number of digits from src to dest
    // This is a function because it's too easy to forget to multiply by the size of the digit.
    void copy_digits(uint32_t* dest, const uint32_t* src, int ndigits) {
        memcpy(dest, src, ndigits * sizeof(uint32_t));
    }
```

This is the minimum set that will cause reallocations, which are the source of most bugs. The
most common user call will be to `resize()`, to grow a `Num` (which can be of arbitrary size).
The rest are the helpers needed to access data without needing to understand how `NumBuffer`
is implemented.

The basic contract is that a `NumBuffer` is an array of `uint32_t`; the start of the array
is given by the `digits()` function, and the length of the array is given by the `length()`
function.

The Num class
=============

```
class Num
{
public:

    [operators]
    [misc]

    NumBuffer data;
```

Since `Num` represents an arbitrary integer value, almost all of the interactions with it are
through operators. Any other interactions with the data are through the `NumBuffer` class,
which encapsulates the storage itself. Unlike primitive numbers, we do need some buffer
management, since a `Num` behaves more like a `string` in that it can be of an arbitrary length.

Note that `Num` does not need any of the big 5: default constructor, destructor, copy
constructor, copy assignment operator, move constructor, or move assignment operator, because
`Num` has no data of its own, just a single instance of `NumData`, which has its own
versions of the big 5.

There are also some extra functions, like `divmod`, that don't map to C++ operators.

We need some way to read and write and compare `Num` values; the natural way is through
strings, but there are some conversions between `Num` and integral types, for convenience.
