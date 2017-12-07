// main.cpp

#include "Num.h"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "../catch.hpp"

#include <cstring>

TEST_CASE("Num - buffer management", "[Num]")
{
    // Test reserve - it should preserve existing data
    Num resizing;
    REQUIRE(resizing.data.local == 1);
    REQUIRE(resizing.small.len == 0);

    resizing.reserve(2);
    REQUIRE(resizing.data.local == 1);
    resizing.small.len = 2;
    resizing.small.buf[0] = 0;
    resizing.small.buf[1] = 1;

    resizing.reserve(8);
    REQUIRE(resizing.data.local == 0);
    REQUIRE(resizing.big.bufsize == 8);
    for (uint32_t i = 0; i < 2; i++)
        REQUIRE(resizing.big.buf[i] == i);
    for (uint32_t i = 0; i < 8; i++)
        resizing.big.buf[i] = i;
    resizing.big.len = 8;

    resizing.reserve(16);
    REQUIRE(resizing.data.local == 0);
    REQUIRE(resizing.big.bufsize == 16);
    for (uint32_t i = 0; i < 8; i++)
        REQUIRE(resizing.big.buf[i] == i);
    for (uint32_t i = 0; i < 16; i++)
        resizing.big.buf[i] = i;
    resizing.big.len = 16;

    // Test grow (internal function)
    Num growv;
    REQUIRE(growv.small.len == 0);
    growv.grow(1);
    REQUIRE(growv.data.local == 1);
    REQUIRE(growv.small.len == 1);
    growv.grow(6);
    REQUIRE(growv.data.local == 1);
    REQUIRE(growv.small.len == 7);
    growv.grow(1);
    REQUIRE(growv.data.local == 0);
    REQUIRE(growv.big.len == 8);
    growv.grow(8);
    REQUIRE(growv.data.local == 0);
    REQUIRE(growv.big.len == 16);

    // Test trim - leave number in canonical form
    {
    Num trimv;
    trimv.reserve(2);
    trimv.databuffer()[0] = 5;
    trimv.databuffer()[1] = 8;
    trimv.small.len = 2;

    trimv.trim();
    REQUIRE(trimv.small.len == 2);
    REQUIRE(trimv.databuffer()[0] == 5);
    REQUIRE(trimv.databuffer()[1] == 8);

    trimv.databuffer()[1] = 0;
    trimv.trim();
    REQUIRE(trimv.small.len == 1);
    REQUIRE(trimv.databuffer()[0] == 5);
    }

    // Test shrink (internal function)
    {
    Num shrinkv;
    REQUIRE(shrinkv.small.len == 0);
    shrinkv.shrink(1);
    REQUIRE(shrinkv.small.len == 0);
    shrinkv.grow(10);
    REQUIRE(shrinkv.big.len == 10);
    shrinkv.shrink(8);
    REQUIRE(shrinkv.big.len == 2);
    }
}

TEST_CASE("Num - big 5 (constructors and copy assignment operators)", "[Num]")
{
    // Basic constructor
    Num zero;
    REQUIRE(zero.data.local == 1);
    REQUIRE(zero.data.len == 0);
    REQUIRE(zero.data.sign == 0);

    // Make some source numbers to test with
    // 16-digit zero
    Num zero16;
    zero16.reserve(16);
    REQUIRE(zero16.data.local == 0);
    REQUIRE(zero16.big.buf != nullptr);
    REQUIRE(zero16.data.len == 0);
    REQUIRE(zero16.big.bufsize == 16);

    // 2-digit nonzero
    Num num2;
    num2.small.buf[0] = 0x0000'000FL;
    num2.small.buf[1] = 0xF000'0000L;
    num2.small.len = 2;
    REQUIRE(num2.data.local == 1);
    REQUIRE(num2.small.len == 2);

    // 16-digit nonzero
    Num num16;
    num16.reserve(16);
    REQUIRE(num16.big.bufsize == 16);
    memset(num16.big.buf, 0, num16.big.bufsize*4);
    num16.big.len = 16;
    num16.big.buf[15] = 0x9988'7766L;
    REQUIRE(num16.data.local == 0);
    REQUIRE(num16.big.len == 16);

    // 8-digit nonzero in 16-digit buffer
    Num num8_16;
    num8_16.reserve(16);
    REQUIRE(num8_16.big.bufsize == 16);
    memset(num8_16.big.buf, 0, num8_16.big.bufsize*4);
    num8_16.big.len = 8;
    num8_16.big.buf[7] = 0x1234'5678L;
    REQUIRE(num8_16.data.local == 0);
    REQUIRE(num8_16.big.len == 8);

    // Copy constructor of big zero - this should produce a small zero
    Num copyzero16{zero16};
    REQUIRE(copyzero16.data.local == 1);
    REQUIRE(copyzero16.small.len == 0);

    // Copy constructor of small num
    Num smallcopy{num2};
    REQUIRE(smallcopy.data.local == 1);
    REQUIRE(smallcopy.small.len == 2);
    REQUIRE(smallcopy.small.buf[0] == 0x0000'000FL);
    REQUIRE(smallcopy.small.buf[1] == 0xF000'0000L);

    // Copy a 8-digit number that is in a 16-digit buffer
    Num bigcopy{num8_16};
    REQUIRE(bigcopy.data.local == 0);
    REQUIRE(bigcopy.big.len == 8);
    REQUIRE(bigcopy.big.bufsize == 8);
    REQUIRE(bigcopy.big.buf[7] == 0x1234'5678L);

    // Copy assign a zero
    Num copyzero = zero;
    REQUIRE(copyzero.data.local == 1);
    REQUIRE(copyzero.small.len == 0);

    // Copy assign a small Num
    Num copysmall = num2;
    REQUIRE(smallcopy.data.local == 1);
    REQUIRE(smallcopy.small.len == 2);
    REQUIRE(smallcopy.small.buf[0] == 0x0000'000FL);
    REQUIRE(smallcopy.small.buf[1] == 0xF000'0000L);

    // Copy assign a small Num into a big one - it stays a big Num
    Num bignum2;
    bignum2.reserve(16);
    bignum2 = num2;
    REQUIRE(bignum2.data.local == 0);
    REQUIRE(bignum2.big.bufsize == 16);
    REQUIRE(bignum2.big.len == 2);
    REQUIRE(bignum2.big.buf[0] == 0x0000'000FL);
    REQUIRE(bignum2.big.buf[1] == 0xF000'0000L);

    // Copy assign a big Num into a big one with a buffer that's too small
    Num bignum8;
    bignum8.reserve(8);
    bignum8 = num16;
    REQUIRE(bignum8.data.local == 0);
    REQUIRE(bignum8.big.bufsize == 16);
    REQUIRE(bignum8.big.len == 16);
    REQUIRE(bignum8.big.buf[15] == 0x9988'7766L);

    // Copy assign a big Num into one with a bigger buffer - the dest
    // retains its bigger buffer
    Num bignum32;
    bignum32.reserve(32);
    bignum32 = num16;
    REQUIRE(bignum32.data.local == 0);
    REQUIRE(bignum32.big.bufsize == 32);
    REQUIRE(bignum32.big.len == 16);
    REQUIRE(bignum32.big.buf[15] == 0x9988'7766L);
}

TEST_CASE("Num - copy assign from primitive numbers", "[Num]")
{
    Num v = 0;
    REQUIRE(v.data.local == 1);
    REQUIRE(v.small.len == 0);

    v = -1;
    REQUIRE(v.data.local == 1);
    REQUIRE(v.small.len == 1);
    REQUIRE(v.small.sign == -1);
    REQUIRE(v.small.buf[0] == 1);

    v = int64_t((1ULL << 63) - 1);
    REQUIRE(v.data.local == 1);
    REQUIRE(v.small.len == 2);
    REQUIRE(v.small.sign == 0);
    REQUIRE(v.small.buf[0] == 0xFFFF'FFFF);
    REQUIRE(v.small.buf[1] == 0x7FFF'FFFF);

    v = uint64_t(-1LL);
    REQUIRE(v.data.local == 1);
    REQUIRE(v.small.len == 2);
    REQUIRE(v.small.sign == 0);
    REQUIRE(v.small.buf[0] == 0xFFFF'FFFF);
    REQUIRE(v.small.buf[1] == 0xFFFF'FFFF);
}

TEST_CASE("Num - addition", "[Num]")
{
    Num augend;
    Num addend;

    // Adding zero
    Num result = augend + addend;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 0);

    result = augend + 0;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 0);

    // positive + positive
    augend = 1;
    addend = 1;
    result = augend + addend;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 1);
    REQUIRE(result.small.sign == 0);
    REQUIRE(result.small.buf[0] == 2);

    // negative + negative
    augend = -1;
    addend = -1;
    result = augend + addend;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 1);
    REQUIRE(result.small.sign == -1);
    REQUIRE(result.small.buf[0] == 2);

    // positive + negative
    augend = 1;
    addend = -1;
    result = augend + addend;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 0);

    augend = 1;
    addend = -2;
    result = augend + addend;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 1);
    REQUIRE(result.small.sign == -1);
    REQUIRE(result.small.buf[0] == 1);
    
    augend = 2;
    addend = -1;
    result = augend + addend;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 1);
    REQUIRE(result.small.sign == 0);
    REQUIRE(result.small.buf[0] == 1);

    // negative + positive
    augend = -1;
    addend = 1;
    result = augend + addend;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 0);

    augend = -1;
    addend = 2;
    result = augend + addend;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 1);
    REQUIRE(result.small.sign == 0);
    REQUIRE(result.small.buf[0] == 1);

    augend = -2;
    addend = 1;
    result = augend + addend;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 1);
    REQUIRE(result.small.sign == -1);
    REQUIRE(result.small.buf[0] == 1);

    // Making a Num bigger than uint64_t and then smaller again
    augend = 0xFFFF'FFFF'FFFF'FFFFULL;
    addend = 1;
    result = augend + addend;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 3);
    REQUIRE(result.small.sign == 0);
    REQUIRE(result.small.buf[0] == 0);
    REQUIRE(result.small.buf[1] == 0);
    REQUIRE(result.small.buf[2] == 1);

    augend = result;
    addend = -1;
    result = augend + addend;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 2);
    REQUIRE(result.small.sign == 0);
    REQUIRE(result.small.buf[0] == 0xFFFF'FFFFUL);
    REQUIRE(result.small.buf[1] == 0xFFFF'FFFFUL);

    augend = 1;
    result = augend + 1;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 1);
    REQUIRE(result.small.sign == 0);
    REQUIRE(result.small.buf[0] == 2);
}

TEST_CASE("Num - multiply and divide", "[Num]")
{
    Num multiplicand;
    Num multiplier;
    Num result;

    result = multiplicand * multiplier;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 0);

    multiplicand = 7;
    multiplier = 6;
    result = multiplicand * multiplier;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 1);
    REQUIRE(result.small.buf[0] == 42);

    multiplicand = 1;
    multiplier = 100;
    for (int i = 0; i < 10; i++)
        multiplicand *= multiplier;
    result = multiplicand;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 3); // 0x56bc75e2d63100000
    REQUIRE(result.small.buf[0] == 0x63100000);
    REQUIRE(result.small.buf[1] == 0x6bc75e2d);
    REQUIRE(result.small.buf[2] == 0x00000005);

    multiplicand = result;
    multiplier = result;
    result = multiplicand * multiplier;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 5); // 0x1d'6329f1c3'5ca4bfab'b9f56100'00000000
    REQUIRE(result.small.buf[0] == 0x00000000);
    REQUIRE(result.small.buf[1] == 0xb9f56100);
    REQUIRE(result.small.buf[2] == 0x5ca4bfab);
    REQUIRE(result.small.buf[3] == 0x6329f1c3);
    REQUIRE(result.small.buf[4] == 0x0000001d);

    // biggest Num in small buf
    // 26959946667150639794667015087019630673637144422540572481103610249216
    // 10^68

    Num dividend;
    Num divisor = 1;

    result = dividend / divisor;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 0);

    dividend = 42;
    divisor = 6;
    result = dividend / divisor;
    REQUIRE(result.data.local == 1);
    REQUIRE(result.small.len == 1);
    REQUIRE(result.small.buf[0] == 7);
}

// old tests
#if 0

TEST_CASE("builtin math", "[C++]")
{
    uint16_t v16 = uint16_t(uint8_t(-1)) * uint16_t(uint8_t(-1));
    REQUIRE(v16 == 0xFE01);
    v16 += uint16_t(uint8_t(-1));
    REQUIRE(v16 == 0xFF00);

    uint32_t v32 = uint32_t(uint16_t(-1)) * uint32_t(uint16_t(-1));
    REQUIRE(v32 == 0xFFFE'0001L);
    uint64_t v64 = uint64_t(uint32_t(-1)) * uint64_t(uint32_t(-1));
    REQUIRE(v64 == 0xFFFF'FFFE'0000'0001LL);
}

TEST_CASE("Num - construction", "[Num]")
{
    Num empty;
    REQUIRE(empty.len() == 0);

    Num twoe32m1{(1LL<<32)-1};
    REQUIRE(reinterpret_cast<uint32_t*>(twoe32m1.raw)[0] == 0x0000'0001);
    REQUIRE(twoe32m1[0] == 0xFFFF'FFFF);
    REQUIRE(twoe32m1.to_int64() == 4294967295);
    REQUIRE(twoe32m1.len() == 1);

    Num twoe63m1{(1ULL<<63)-1LL};
    REQUIRE(twoe63m1.to_int64() == 0x7FFF'FFFF'FFFF'FFFFLL);
    REQUIRE(twoe63m1.len() == 2);

    Num neg1{-1};
    REQUIRE(reinterpret_cast<uint32_t*>(neg1.raw)[0] == 0xFFFF'0001);
    REQUIRE(neg1[0] == 0x0000'0001);
    REQUIRE(neg1.to_int64() == -1);
    REQUIRE(neg1.len() == 1);
}

TEST_CASE("Num - util", "[Num]")
{
    Num zero{0LL};
    REQUIRE(zero.len() == 0);

    Num pos{1};
    REQUIRE(pos.len() == 1);
    REQUIRE(pos.sign() >= 0);

    Num neg{-1};
    REQUIRE(neg.len() == 1);
    REQUIRE(neg.sign() < 0);
}

TEST_CASE("Num - addition", "[Num]")
{
    // Start out with a nearly trivial number
    Num v{1};
    REQUIRE(v.len() == 1);
    REQUIRE(v[0] == 1);

    // Double it
    v += v;
    REQUIRE(v.len() == 1);
    REQUIRE(v[0] == 2);

    // Grow the number to 2^31, which still fits in a single digit
    for (int i = 0; i < 30; i++)
        v += v;
    REQUIRE(v.len() == 1);
    REQUIRE(v[0] == 0x8000'0000L);

    // Grow the number to 2^32, which spills over into a second digit
    v += v;
    REQUIRE(v.len() == 2);
    REQUIRE(v[0] == 0);
    REQUIRE(v[1] == 1);

    // Grow the number to 2^63 and then 2^64
    for (int i = 0; i < 31; i++)
        v += v;
    REQUIRE(v.len() == 2);
    REQUIRE(v[0] == 0);
    REQUIRE(v[1] == 0x8000'0000L);

    v += v;
    REQUIRE(v.len() == 3);
    REQUIRE(v[0] == 0);
    REQUIRE(v[1] == 0);
    REQUIRE(v[2] == 1);

    // Now grow it to 2^223, which is the largest power of 2
    // we can fit in our SOO static data
    for (int i = 0; i < 159; i++)
        v += v;
    REQUIRE(v.len() == 7);
    REQUIRE(v[6] == 0x8000'0000L);

    // Add positive and negative numbers together
    Num vp{100};
    Num vn{-100};
    v = vp + vn;
    REQUIRE(v.len() == 0);
    REQUIRE(v.sign() >= 0);

    v = vn + vp;
    REQUIRE(v.len() == 0);
    REQUIRE(v.sign() >= 0);

    vp = 101;
    v = vp + vn;
    REQUIRE(v.len() == 1);
    REQUIRE(v.sign() >= 0);
    REQUIRE(v[0] == 1);

    vn = -102;
    v = vp + vn;
    REQUIRE(v.len() == 1);
    REQUIRE(v.sign() < 0);
    REQUIRE(v[0] == 1);

    v = vn + vn;
    REQUIRE(v.len() == 1);
    REQUIRE(v.sign() < 0);
    REQUIRE(v[0] == 204);
}

TEST_CASE("Num - subtraction", "[Num]")
{
    Num v;
    Num v1;
    Num v2;

    // subtract positive and negative numbers
    v1 = 1;
    v2 = -1;
    v = v1 - v2;
    REQUIRE(v.len() == 1);
    REQUIRE(v.sign() >= 0);
    REQUIRE(v[0] == 2);

    v2 = 1;
    v = v1 - v2;
    REQUIRE(v.len() == 0);
    REQUIRE(v.sign() >= 0);

    v1 = -1;
    v = v1 - v2;
    REQUIRE(v.len() == 1);
    REQUIRE(v.sign() < 0);
    REQUIRE(v[0] == 2);

    v2 = -1;
    v = v1 - v2;
    REQUIRE(v.len() == 0);
    REQUIRE(v.sign() >= 0);
}

TEST_CASE("Low-level divide", "[Num]")
{
    uint32_t dividend[4];
    uint32_t divisor[4];
    uint32_t quotient[4];
    uint32_t remainder[4];

    dividend[0] = 11;
    divisor[0] = 2;
    int m = 1;
    int n = 1;

    bool ok = MultiwordDivide<uint32_t>(quotient, remainder, dividend, divisor, m, n);
    REQUIRE(ok);
    REQUIRE(quotient[0] == 5);
    REQUIRE(remainder[0] == 1);

    dividend[0] = 0;
    dividend[1] = 16;
    divisor[0] = 1;
    divisor[1] = 1;
    m = 2;
    n = 2;

    ok = MultiwordDivide<uint32_t>(quotient, remainder, dividend, divisor, m, n);
    REQUIRE(ok);
    REQUIRE(quotient[0] == 15);
    REQUIRE(remainder[0] == 0xFFFF'FFF1);
}

TEST_CASE("Num - division", "[Num]")
{
    Num v;
    Num v1;
    Num v2;

    v1 = 32;
    v2 = 16;
    v = v1 / v2;
    REQUIRE(v.len() == 1);
    REQUIRE(v[0] == 2);

    v1 = 100;
    v2 = 3;
    v = v1 / v2;
    REQUIRE(v.len() == 1);
    REQUIRE(v[0] == 33);

#if 0
    v1 = Num{1'000'000'000'000LL} * Num{1'000'000'000'000LL};
    v2 = Num{1'000'000'000} * Num{1'000'000'000};
    v = v1 / v2;
    REQUIRE(v.len() == 1);
    REQUIRE(v[0] == 1'000'000);
#endif
}

TEST_CASE("Num - multiplication", "[Num]")
{
    Num v;
    Num v1;
    Num v2;

    v1 = 1;
    v2 = 1;
    v = v1 * v2;
    REQUIRE(v.len() == 1);
    REQUIRE(v[0] == 1);

    v1 = 0xFFFF'FFFFL;
    v2 = 0xFFFF'FFFFL;
    v = v1 * v2;
    REQUIRE(v.len() == 2);
    REQUIRE(v[0] == 0x0000'0001L);
    REQUIRE(v[1] == 0xFFFF'FFFEL);
}

#if 0
TEST_CASE("Soak test", "[Num]")
{
    //uint64_t imax = uint64_t(0x7F00'0000'0000'0000LL);
    uint64_t imax = 50000;
    uint64_t iinc = 1;
    uint64_t jinc = 1;

    int status = 0;
    for (uint64_t i = 0; i < imax; i += iinc)
    {
        for (uint64_t j = 0; j < imax; j += jinc)
        {
            Num I{(long long) i};
            Num J{(long long) j};
            Num K;

            K = I + J;
            if (i+j == 0)
                REQUIRE(K.len() == 0);
            else
                REQUIRE(K[0] == i+j);

            K = I - J;
            if (i == j)
                REQUIRE(K.len() == 0);
            else if (i >= j)
                REQUIRE(K[0] == i-j);
            else
                REQUIRE(K[0] == j-i);

            K = I * J;
            if (i*j == 0)
                REQUIRE(K.len() == 0);
            else
                REQUIRE(K[0] == i*j);

            if (j != 0)
            {
                K = I / J;
                if (i/j == 0)
                    REQUIRE(K.len() == 0);
                else
                    REQUIRE(K[0] == i/j);
            }

            status += 1;
            if (status == 50000)
            {
                std::cout << i << "," << j << "\n";
                status = 0;
            }
        }
    }
}
#endif

TEST_CASE("Num - to/from cstring", "[Num]")
{
    Num v;
    char s[128];
    int n;

    n = v.to_cstring(s, 2);
    REQUIRE(n == 2);
    REQUIRE(strlen(s) == 1);
    REQUIRE(s[0] == '0');
    REQUIRE(s[1] == 0);

    v = 1;
    n = v.to_cstring(s, 2);
    REQUIRE(n == 2);
    REQUIRE(strlen(s) == 1);
    REQUIRE(s[0] == '1');
    REQUIRE(s[1] == 0);

    v = -1;
    n = v.to_cstring(s, 3);
    REQUIRE(n == 3);
    REQUIRE(strlen(s) == 2);
    REQUIRE(0 == strcmp(s, "-1"));

    v = 1'000'000;
    n = v.to_cstring(s, 8);
    REQUIRE(n == 8);
    REQUIRE(0 == strcmp(s, "1000000"));

    v = -1'000'000;
    n = v.to_cstring(s, 9);
    REQUIRE(n == 9);
    REQUIRE(0 == strcmp(s, "-1000000"));

    v = Num{0x7FFF'FFFF'FFFF'FFFFLL} * Num{0x7FFF'FFFF'FFFF'FFFFLL};
    n = v.to_cstring(s, 39);
    REQUIRE(n == 39);
    REQUIRE(0 == strcmp(s, "85070591730234615847396907784232501249"));
}

TEST_CASE("Mul", "[Num]")
{
    Num v;
    Num n256{256};
    Num n255{255};

    char* vals[] = {
        "0",
        "255",
        "65535",
        "16777215",
        "4294967295",
        "1099511627775",
        "281474976710655",
        "72057594037927935",
        "18446744073709551615",
        "4722366482869645213695",
        "1208925819614629174706175",
        "309485009821345068724781055",
        "79228162514264337593543950335",
        "20282409603651670423947251286015", // 0xFF'FFFF'FFFF'FFFF'FFFF'FFFF'FFFF
        0
    };

    uint64_t nums[] = {
        0,
        255U,
        65'535U,
        16'777'215UL,
        4'294'967'295UL,
        1'099'511'627'775ULL,
        281'474'976'710'655ULL,
        72'057'594'037'927'935ULL,
        18'446'744'073'709'551'615ULL,
        0, // will no longer fit in a uint64_t
        0,
        0,
        0,
        0
    };

    for (int i = 0; vals[i] != nullptr; i++)
    {
        char buf[256];
        int l = v.to_cstring(buf, 256);
        std::cout << buf << "\n";
        REQUIRE(l < 256);

        if (nums[i] != 0)
            REQUIRE(nums[i] == v.to_uint64());
        REQUIRE(0 == strcmp(vals[i], buf));

        v = v*n256 + n255;
    }
}

TEST_CASE("Num - divmod", "[Num]")
{
    Num v;
    Num q;
    Num r;
    Num n10{10};
    Num n256{256};
    Num n255{255};
    Num qdec;
    for (int i = 0; i < 13; i++)
    {
        char buf[256];
        int clen = v.to_cstring(buf, 256);
        REQUIRE(clen < sizeof(buf));
        std::cout << "v=" << buf;

        v.divmod(n10, q, r);
        uint32_t rdec = v.divmod(10, qdec);
        char qbuf[256];
        clen = qdec.to_cstring(qbuf, 256);
        REQUIRE(clen < sizeof(buf));

        REQUIRE(rdec == r.to_int64());
        REQUIRE(q.to_int64() == qdec.to_int64());
        std::cout << " q=" << qbuf << " r=" << rdec << "\n";

        v = v*n256 + n255;
    }
}

TEST_CASE("Num - to/from string", "[Num]")
{
    char buf[256];
    Num v("123456");
    v.to_cstring(buf, 256);
    REQUIRE(0 == strcmp(buf, "123456"));

    v = Num{"FFFF", 16};
    v.to_cstring(buf, 256);
    REQUIRE(0 == strcmp(buf, "65535"));

    char mbuf[256];
    mbuf[0] = '1';
    mbuf[1] = 0;
    v = 1;
    for (int i = 0; i < 60; i++)
    {
        std::cout << mbuf << "\n";
        v.to_cstring(buf, 256);
        REQUIRE(0 == strcmp(buf, mbuf));
        v *= 10;
        strcat(mbuf, "0");
    }
}
#endif
