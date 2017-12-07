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

The Num class
=============

```
class Num
{
public:
    Num();

    // Big 5: destructor, copy constructor, copy assignment operator, move constructor,
    // move assignment operator.
    ~Num() noexcept;
    Num(const Num& other) noexcept;            // Copy constructor
    Num& operator=(const Num& other) noexcept; // Copy assignment operator

    [buffer management]
    [operators]
    [misc]
```

Since `Num` represents an arbitrary integer value, almost all of the interactions with it are
through operators.

Unlike primitive numbers, we do need some buffer management, since a `Num` behaves more like
a `string` in that it can be of an arbitrary length.

There are also some extra functions, like `divmod`, that don't map to C++ operators.

We need some way to read and write and compare `Num` values; the natural way is through
strings, but there are some conversions between `Num` and integral types, for convenience.
