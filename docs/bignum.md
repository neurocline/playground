# Multiple precision math

In the past 40 years, we have made big advances. Early processors had 8-bit add 
and subtract, and we had to build everything up from those primitives. A 16x16
multiple could take hundreds of clock cycles, and division thousands. Now we
have 64-bit processors with a 128/64 divide instruction that takes a few hundred
clock cycles at a 3000x cycle rate. And yet it's never enough.

It's not common, but sometimes we need more precision; we need to compute
something large, and we can't afford the imprecise results that a 64-bit floating
point number gives us.

# Reference

Overview

- [Arbitrary-precision arithmetic](https://en.wikipedia.org/wiki/Arbitrary-precision_arithmetic)
- [Computational complexity of mathematical operations](https://en.wikipedia.org/wiki/Computational_complexity_of_mathematical_operations)
- [List of arbitrary-precision arithmetic software](https://en.wikipedia.org/wiki/List_of_arbitrary-precision_arithmetic_software)

GNU MP Bignum Library and derivatives

- [GNU MP Bignum Library](https://gmplib.org/)
- [GNU Multiple Precision Arithmetic Library](https://en.wikipedia.org/wiki/GNU_Multiple_Precision_Arithmetic_Library)
- [MPIR: Multiple Precision Integers and Rationals](http://mpir.org/)
- [MPIR](https://en.wikipedia.org/wiki/MPIR_(mathematics_software))
- [GNU MPFR](https://en.wikipedia.org/wiki/GNU_MPFR)
- [Class Library for Numbers](https://en.wikipedia.org/wiki/Class_Library_for_Numbers)
- [GMP Algorithms](https://gmplib.org/manual/Algorithms.html#Algorithms)

LibTomMath

- [libtom/libtommath](https://github.com/libtom/libtommath)
- [LibTom projects](http://www.libtom.net/)
- [LibTomMath](http://www.libtom.net/LibTomMath/)

Misc

- [BSDNT - BSD Licensed Bignum Library](https://github.com/wbhart/bsdnt)
- [C++ Big Integer Library](https://mattmccutchen.net/bigint/)
- [ttmath](https://github.com/logtcn/ttmath)
- [Infint](https://sercantutar.github.io/infint/)
- [Huge Long Numbers](https://github.com/daidodo/huge-long-number)
- [A Class for Representing Large Numbers](http://www.drdobbs.com/a-class-for-representing-large-integers/184403247)
- [Multiprecision arithmetic](https://cryptojedi.org/peter/data/space-20141020.pdf)

More

- [A survey of fast exponentiation methods](https://pdfs.semanticscholar.org/482d/a61cfb06aaad135bd0df0c367d2bbec0686b.pdf)
