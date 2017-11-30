// main.cpp

#include "Num.h"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "../catch.hpp"

#include <cstring>

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

#if 0

#include "Num.h"

#include <assert.h>

int main(int /*argc*/, char** /*argv*/)
{
    Num divisor{0LL};
    Num dividend{0LL};

    for (int i = 0; i < 100; i++)
    {
        divisor = divisor + 11;
        for (int j = 0; j < 100; j++)
        {
            dividend = dividend + 29;
            // Num temp = dividend / divisor;
        }
    }

    long long expect = 100*100*29;
    long long actual = dividend.to_int64();
    assert(expect == actual);

    Num qb{0x12345678L};
    uint32_t v = qb[0];
    assert(v == 0x12345678L);
    assert(qb.len() < 2 || qb[1] == 0);
    Num qq{0x1122334455667788LL};
    assert(qq[0] == 0x55667788L);
    assert(qq[1] == 0x11223344L);
    return 0;
}

#if 0

#include "Bignum.h"

#include <cassert>
#include <iostream>

// dividend = 676201909253
// divisor = 993319
// correct = 680750
// returned = 680749

// dividend = 131075
// divisor = 131075
// obviously this should be 1
// returned 0

int main(int /*argc*/, char** /*argv*/)
{
    Bignum b("102030405060708090");
    TestBignum();

    //for (uint64_t visor = 993319; visor < 1ULL << 48; visor += 993319)
    //    for (uint64_t dend = 1046527; dend < 1ULL << 48; dend += 1046527)
    int count = 0;
    for (uint64_t dend = 1ULL<<33; dend < 1ULL << 48; dend += 1)
        for (uint64_t visor = 1; visor <= dend; visor += 1)
        {
#if 0
            uint16_t dividend[4];
            uint16_t divisor[4];

            #define ASSIGN(T, V) \
                { uint64_t v = V; \
                T[0] = (v & 0xFFFF); v >>= 16; \
                T[1] = (v & 0xFFFF); v >>= 16; \
                T[2] = (v & 0xFFFF); v >>= 16; \
                T[3] = (v & 0xFFFF); }

            ASSIGN(dividend, dend)
            ASSIGN(divisor, visor)

            #define SIZE(T) \
                T[3] != 0 ? 4 : T[2] != 0 ? 3 : T[1] != 0 ? 2 : 1

            int m = SIZE(dividend);
            int n = SIZE(divisor);

            uint16_t quotient[4];
            uint16_t remainder[4];

            //bool ok = LongDivide(quotient, remainder, dividend, divisor, m, n);
            bool ok = MultiwordDivide<uint16_t>(quotient, remainder, dividend, divisor, m, n);

            assert(ok);

            #define FETCH(T) \
                (uint64_t(T[0]) | (uint64_t(T[1]) << 16) | (uint64_t(T[2]) << 32) | (uint64_t(T[3]) << 48))

            uint64_t q = FETCH(quotient);
            uint64_t qf = dend / visor;
            assert(q == qf);
#else
            uint32_t dividend[2];
            uint32_t divisor[2];

            #define ASSIGN(T, V) \
                { uint64_t v = V; \
                T[0] = (v & 0xFFFFFFFF); v >>= 32; \
                T[1] = (v & 0xFFFFFFFF); }

            ASSIGN(dividend, dend)
            ASSIGN(divisor, visor)

            #define SIZE(T) \
                T[1] != 0 ? 2 : 1

            int m = SIZE(dividend);
            int n = SIZE(divisor);

            uint32_t quotient[2]; quotient[1] = 0;
            uint32_t remainder[2]; remainder[1] = 0;

            bool ok = MultiwordDivide<uint32_t>(quotient, remainder, dividend, divisor, m, n);

            assert(ok);

            #define FETCH(T) \
                (uint64_t(T[0]) | (uint64_t(T[1]) << 32))

            uint64_t q = FETCH(quotient);
            uint64_t qf = dend / visor;
            assert(q == qf);
            uint64_t r = FETCH(remainder);
            assert(dend == q * visor + r);
#endif

            count++;
            if (count == 25'000'000)
            {
                std::cout << dend << '/' << visor << " = " << q << ", " << r << std::endl;
                count = 0;
            }
        }
}

// 676201909250
// 676201909253

#endif //

#endif

