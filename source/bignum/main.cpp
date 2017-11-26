// main.cpp

#include "Num.h"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "../catch.hpp"

TEST_CASE("Num - construction", "[Num]")
{
    Num empty;
    REQUIRE(empty.len() == 0);

    Num twoe32m1{(1LL<<32)-1};
    REQUIRE(reinterpret_cast<uint32_t*>(twoe32m1.raw)[0] == 0x0000'0001);
    REQUIRE(twoe32m1[0] == 0xFFFF'FFFF);
    REQUIRE(twoe32m1.to_int64() == 4294967295);

    Num twoe63m1{(1LL<<63)-1};
    REQUIRE(twoe63m1.to_int64() == 0x7FFF'FFFF'FFFF'FFFFLL);

    Num neg1{-1};
    REQUIRE(reinterpret_cast<uint32_t*>(neg1.raw)[0] == 0xFFFF'0001);
    REQUIRE(neg1[0] == 0x0000'0001);
    REQUIRE(neg1.to_int64() == -1);
}

TEST_CASE("Num - addition", "[Num]")
{
    Num one{1};
    Num two{2};
    Num three = one + two;
    REQUIRE(three.to_int64() == 3);

    Num accum; // ugg, fix "ambiguous Num{0}" issue
    Num delta{1021};
    for (int i = 0; i < 1000; i++)
        accum = accum + delta;
    REQUIRE(accum.to_int64() == 1021*1000);
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

