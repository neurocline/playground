// main.cpp

#include "Num.h"

#include <cstring>
#include <ctime>

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "../catch.hpp"

TEST_CASE("NumBuffer - construct/destruct", "[NumBuffer]")
{
    SECTION("Simple default constructor")
    {
        NumBuffer buf;
        REQUIRE_FALSE(buf.nonlocal);
        REQUIRE(buf.sign == 0);
        REQUIRE(buf.len == 0);
    }

    // Placement new should leave the internal buffer uninitialized, since the
    // NumBuffer is empty.
    SECTION("Placement new default constructor")
    {
        char v[sizeof(NumBuffer)];
        memset(v, 255, sizeof(NumBuffer));
        NumBuffer* buf = new(v) NumBuffer;

        REQUIRE_FALSE(buf->nonlocal);
        REQUIRE(buf->sign == 0);
        REQUIRE(buf->len == 0);
        REQUIRE(buf->buf[0] == 0xFFFFFFFF);
        REQUIRE(buf->buf[NumBuffer::smallbufsize-1] == 0xFFFFFFFF);

        // Destructor should put the object in an invalid state as well as removing
        // any allocated memory
        buf->nonlocal = 1;
        buf->big.digits = new uint32_t[16];
        buf->big.bufsize = 16;
        buf->len = 16;

        buf->~NumBuffer();
        REQUIRE(buf->nonlocal);
        REQUIRE(buf->big.digits == nullptr);
    }

    SECTION("Testing destructor")
    {
        NumBuffer buf;

        buf.reserve(16);
        REQUIRE(buf.nonlocal);
        REQUIRE(buf.big.bufsize == 16);
        REQUIRE(buf.len == 0);
        REQUIRE(buf.big.digits != nullptr);
        buf.~NumBuffer();
        REQUIRE(buf.big.digits == nullptr);
    }
}

TEST_CASE("NumBuffer - buffer management", "[NumBuffer]")
{
    SECTION("Testing reserve")
    {
        // This is a small object
        NumBuffer buf;
        buf.reserve(1);
        REQUIRE_FALSE(buf.nonlocal);
        REQUIRE(buf.sign == 0);
        REQUIRE(buf.len == 0);

        buf.len = 1;
        buf.buf[0] = 20;
        REQUIRE(buf.len == 1);
        REQUIRE(buf.buf[0] == 20);

        // This is a small object
        buf.reserve(2);
        REQUIRE_FALSE(buf.nonlocal);
        REQUIRE(buf.len == 1);
        REQUIRE(buf.buf[0] == 20);
        buf.len = 2;
        buf.buf[1] = 21;

        // This is the largest small object
        buf.reserve(NumBuffer::smallbufsize);
        REQUIRE_FALSE(buf.nonlocal);
        REQUIRE(buf.len == 2);
        REQUIRE(buf.buf[0] == 20);
        REQUIRE(buf.buf[1] == 21);
        buf.len = NumBuffer::smallbufsize;
        buf.buf[NumBuffer::smallbufsize-1] = 29;

        // This just became a big object
        buf.reserve(NumBuffer::smallbufsize+1);
        REQUIRE(buf.nonlocal);
        REQUIRE(buf.len == NumBuffer::smallbufsize);
        REQUIRE(buf.big.digits[0] == 20);
        REQUIRE(buf.big.digits[NumBuffer::smallbufsize-1] == 29);
        buf.len = NumBuffer::smallbufsize+1;
        buf.big.digits[NumBuffer::smallbufsize] = 30;

        // This is a big object
        buf.reserve(100);
        REQUIRE(buf.nonlocal);
        REQUIRE(buf.len == NumBuffer::smallbufsize+1);
        REQUIRE(buf.big.digits[0] == 20);
        REQUIRE(buf.big.digits[NumBuffer::smallbufsize] == 30);
    }

    SECTION("Testing resize")
    {
        NumBuffer buf;
        buf.resize(7);
        REQUIRE_FALSE(buf.nonlocal);
        REQUIRE(buf.sign == 0);
        REQUIRE(buf.len == 7);

        buf.buf[0] = 1;
        buf.buf[6] = 7;
        buf.resize(8);
        REQUIRE(buf.nonlocal);
        REQUIRE(buf.len == 8);
        REQUIRE(buf.big.bufsize == 10);
        REQUIRE(buf.big.digits[0] == 1);
        REQUIRE(buf.big.digits[6] == 7);
        buf.big.digits[7] = 8;
    }
}

TEST_CASE("NumBuffer - copy/move", "[NumBuffer]")
{
    // Create a small object of length 1
    NumBuffer small;
    small.reserve(1);
    small.len = 1;
    small.buf[0] = 20;

    // Create a large object of length 10
    NumBuffer large;
    large.reserve(10);
    large.len = 10;
    memset(large.big.digits, 0x66, 10*sizeof(uint32_t));

    SECTION("Testing copy constructor of small object")
    {
        // Copy construct from a small object
        NumBuffer buf(small);
        REQUIRE_FALSE(buf.nonlocal);
        REQUIRE(buf.len == 1);
        REQUIRE(buf.buf[0] == 20);
    }

    SECTION("Testing copy constructor of large object")
    {
        // Copy construct from a large object
        NumBuffer buf(large);
        REQUIRE(buf.nonlocal);
        REQUIRE(buf.len == 10);
        REQUIRE(buf.big.digits[0] == 0x66666666);
        REQUIRE(buf.big.digits[9] == 0x66666666);
    }

    SECTION("Testing move constructor of small object")
    {
        NumBuffer buf(std::move(small));
        REQUIRE_FALSE(buf.nonlocal);
        REQUIRE(buf.len == 1);
        REQUIRE(buf.buf[0] == 20);

        REQUIRE(small.nonlocal);
        REQUIRE(small.big.digits == nullptr);
    }

    SECTION("Testing move constructor of large object")
    {
        NumBuffer buf(std::move(large));
        REQUIRE(buf.nonlocal);
        REQUIRE(buf.len == 10);
        REQUIRE(buf.big.digits[0] == 0x66666666);
        REQUIRE(buf.big.digits[9] == 0x66666666);

        REQUIRE(large.nonlocal);
        REQUIRE(large.big.digits == nullptr);
    }

    SECTION("Testing copy assignment operator")
    {
        NumBuffer buf;

        // Small = small
        buf.resize(NumBuffer::smallbufsize);
        memset(buf.buf, 0x42, NumBuffer::smallbufsize*sizeof(uint32_t));
        buf = small;
        REQUIRE_FALSE(buf.nonlocal);
        REQUIRE(buf.len == 1);
        REQUIRE(buf.buf[0] == 20);
        REQUIRE(buf.buf[1] == 0x42424242);

        // Small = large
        buf = large;
        REQUIRE(buf.nonlocal);
        REQUIRE(buf.len == 10);
        REQUIRE(buf.big.digits[0] == 0x66666666);
        REQUIRE(buf.big.digits[9] == 0x66666666);

        // large = small
        buf.resize(20);
        memset(buf.big.digits, 0x42, 20*sizeof(uint32_t));
        buf = small;
        REQUIRE(buf.nonlocal);
        REQUIRE(buf.len == 1);
        REQUIRE(buf.big.digits[0] == 20);

        // large = large
        buf.resize(20);
        buf = large;
        REQUIRE(buf.nonlocal);
        REQUIRE(buf.len == 10);
        REQUIRE(buf.big.digits[0] == 0x66666666);
        REQUIRE(buf.big.digits[9] == 0x66666666);
    }

    SECTION("Testing move assignment of small object")
    {
        NumBuffer buf;
        buf = std::move(small);
        REQUIRE_FALSE(buf.nonlocal);
        REQUIRE(buf.len == 1);
        REQUIRE(buf.buf[0] == 20);

        REQUIRE(small.nonlocal);
        REQUIRE(small.big.digits == nullptr);
    }

    SECTION("Testing move assignment of large object")
    {
        NumBuffer buf;
        buf = std::move(large);
        REQUIRE(buf.nonlocal);
        REQUIRE(buf.len == 10);
        REQUIRE(buf.big.digits[0] == 0x66666666);
        REQUIRE(buf.big.digits[9] == 0x66666666);

        REQUIRE(large.nonlocal);
        REQUIRE(large.big.digits == nullptr);
    }
}

TEST_CASE("Num - copy assign from primitive numbers", "[Num]")
{
    Num v = 0;
    REQUIRE_FALSE(v.data.nonlocal);
    REQUIRE(v.data.len == 0);

    v = -1;
    REQUIRE_FALSE(v.data.nonlocal);
    REQUIRE(v.data.len == 1);
    REQUIRE(v.data.sign == -1);
    REQUIRE(v.data.buf[0] == 1);

    v = int64_t((1ULL << 63) - 1);
    REQUIRE_FALSE(v.data.nonlocal);
    REQUIRE(v.data.len == 2);
    REQUIRE(v.data.sign == 0);
    REQUIRE(v.data.buf[0] == 0xFFFF'FFFF);
    REQUIRE(v.data.buf[1] == 0x7FFF'FFFF);

    v = uint64_t(-1LL);
    REQUIRE_FALSE(v.data.nonlocal);
    REQUIRE(v.data.len == 2);
    REQUIRE(v.data.sign == 0);
    REQUIRE(v.data.buf[0] == 0xFFFF'FFFF);
    REQUIRE(v.data.buf[1] == 0xFFFF'FFFF);
}

TEST_CASE("Num - simple addition", "[Num]")
{
    Num zero{0};
    REQUIRE(zero.data.len == 0);
    Num plusone{1};
    REQUIRE(plusone.data.len == 1);
    REQUIRE(plusone.data.sign == 0);
    Num plustwo{2};
    REQUIRE(plustwo.data.len == 1);
    REQUIRE(plustwo.data.sign == 0);
    REQUIRE(plustwo.data.buf[0] == 2);
    Num minusone{-1};
    REQUIRE(minusone.data.len == 1);
    REQUIRE(minusone.data.sign == -1);
    Num minustwo{-2};
    REQUIRE(minustwo.data.len == 1);
    REQUIRE(minustwo.data.sign == -1);
    REQUIRE(minustwo.data.buf[0] == 2);

    SECTION("Adding zero")
    {
        Num result = zero + zero;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);

        result = zero;
        result += zero;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);

        result = plusone + zero;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 1);

        result = minusone + zero;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 1);
    }

    SECTION("Adding positive and positive")
    {
        Num result = plusone + plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 2);

        result = plusone;
        result += plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 2);

        result = plustwo + plustwo;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 4);
    }

    SECTION("Adding positive and negative")
    {
        Num result = plusone + minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = plusone;
        result += minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = plusone + minustwo;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 1);

        result = plustwo + minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 1);
    }

    SECTION("Adding negative and positive")
    {
        Num result = minusone + plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = minusone;
        result += plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = minusone + plustwo;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 1);

        result = minustwo + plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 1);
    }

    SECTION("Adding negative and negative")
    {
        Num result = minusone + minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 2);

        result = minusone;
        result += minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 2);

        result = minusone + minustwo;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 3);

        result = minustwo + minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 3);
    }

    SECTION("Adding over a digit boundary")
    {
        // Add longer value to shorter value
        Num result = 0xFFFF'FFFF'FFFF'FFFFULL;
        REQUIRE(result.data.len == 2);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.buf[1] == 0xFFFF'FFFFUL);

        result += plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 3);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0);
        REQUIRE(result.data.buf[1] == 0);
        REQUIRE(result.data.buf[2] == 1);

        result += minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 2);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.buf[1] == 0xFFFF'FFFFUL);

        // Now add shorter value to longer value
        result = plusone + result;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 3);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0);
        REQUIRE(result.data.buf[1] == 0);
        REQUIRE(result.data.buf[2] == 1);

        result = minusone + result;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 2);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.buf[1] == 0xFFFF'FFFFUL);
    }
}

TEST_CASE("Num - simple subtraction", "[Num]")
{
    Num zero{0};
    Num plusone{1};
    Num plustwo{2};
    Num minusone{-1};
    Num minustwo{-2};

    SECTION("Subtracting zero")
    {
        Num result = zero - zero;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);

        result = zero;
        result -= zero;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);

        result = plusone - zero;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 1);

        result = minusone - zero;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 1);
    }

    SECTION("Subtracting positive and positive")
    {
        Num result = plusone - plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = plusone;
        result -= plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = plusone - plustwo;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 1);

        result = plustwo - plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 1);

        result = plustwo - plustwo;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);
    }

    SECTION("Subtracting positive and negative")
    {
        Num result = plusone + minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = plusone;
        result += minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = plusone + minustwo;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 1);

        result = plustwo + minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 1);
    }

    SECTION("Subtracting negative and positive")
    {
        Num result = minusone + plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = minusone;
        result += plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = minusone + plustwo;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 1);

        result = minustwo + plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 1);
    }

    SECTION("Subtracting negative and negative")
    {
        Num result = minusone + minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 2);

        result = minusone;
        result += minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 2);

        result = minusone + minustwo;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 3);

        result = minustwo + minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 1);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 3);
    }

    SECTION("Subtracting over a digit boundary")
    {
        // Add longer value to shorter value
        Num result = 0xFFFF'FFFF'FFFF'FFFFULL;
        REQUIRE(result.data.len == 2);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.buf[1] == 0xFFFF'FFFFUL);

        result += plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 3);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0);
        REQUIRE(result.data.buf[1] == 0);
        REQUIRE(result.data.buf[2] == 1);

        result += minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 2);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.buf[1] == 0xFFFF'FFFFUL);

        // Now add shorter value to longer value
        result = plusone + result;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 3);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0);
        REQUIRE(result.data.buf[1] == 0);
        REQUIRE(result.data.buf[2] == 1);

        result = minusone + result;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 2);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.buf[1] == 0xFFFF'FFFFUL);
    }
}

TEST_CASE("Num - multiply", "[Num]")
{
    Num zero{0};
    Num plusone{1};
    Num plustwo{2};
    Num minusone{-1};
    Num minustwo{-2};
    Num pluslarge{0xFFFF'FFFF'FFFF'FFFFULL};

    SECTION("Multiplying by zero")
    {
        Num result = zero * zero;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = zero * plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = plusone * zero;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = zero * minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = minusone * zero;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = pluslarge * zero;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);

        result = zero * pluslarge;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 0);
        REQUIRE(result.data.sign == 0);
    }

    SECTION("Multiply large by small")
    {
        Num result = pluslarge * plusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 2);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.buf[1] == 0xFFFF'FFFFUL);

        result = pluslarge * minusone;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 2);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.buf[1] == 0xFFFF'FFFFUL);

        result = pluslarge * plustwo;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 3);
        REQUIRE(result.data.buf[0] == 0xFFFF'FFFEUL);
        REQUIRE(result.data.buf[1] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.buf[2] == 1);

        result = plusone * pluslarge;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 2);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.buf[1] == 0xFFFF'FFFFUL);

        result = minusone * pluslarge;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 2);
        REQUIRE(result.data.sign == -1);
        REQUIRE(result.data.buf[0] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.buf[1] == 0xFFFF'FFFFUL);

        result = plustwo * pluslarge;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 3);
        REQUIRE(result.data.buf[0] == 0xFFFF'FFFEUL);
        REQUIRE(result.data.buf[1] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.buf[2] == 1);
    }

    SECTION("Multiply large by large")
    {
        // (2^64 - 1) * (2^64 - 1)
        Num result = pluslarge * pluslarge;
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 4);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.buf[0] == 0x0000'0001UL);
        REQUIRE(result.data.buf[1] == 0x0000'0000UL);
        REQUIRE(result.data.buf[2] == 0xFFFF'FFFEUL);
        REQUIRE(result.data.buf[3] == 0xFFFF'FFFFUL);

        // (2^128 - 1) * (2^128 - 1) exceeds the small bufffer
        memset(result.data.buf, 255, 16);
        result = result * result;
        REQUIRE(result.data.nonlocal);
        REQUIRE(result.data.len == 8);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.big.digits[0] == 0x0000'0001UL);
        REQUIRE(result.data.big.digits[1] == 0x0000'0000UL);
        REQUIRE(result.data.big.digits[2] == 0x0000'0000UL);
        REQUIRE(result.data.big.digits[3] == 0x0000'0000UL);
        REQUIRE(result.data.big.digits[4] == 0xFFFF'FFFEUL);
        REQUIRE(result.data.big.digits[5] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.big.digits[6] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.big.digits[7] == 0xFFFF'FFFFUL);

        // (2^256 - 1) * (2^256 - 1) is even bigger
        memset(result.data.big.digits, 255, 32);
        result = result * result;
        REQUIRE(result.data.nonlocal);
        REQUIRE(result.data.len == 16);
        REQUIRE(result.data.sign == 0);
        REQUIRE(result.data.big.digits[0] == 0x0000'0001UL);
        REQUIRE(result.data.big.digits[1] == 0x0000'0000UL);
        REQUIRE(result.data.big.digits[2] == 0x0000'0000UL);
        REQUIRE(result.data.big.digits[3] == 0x0000'0000UL);
        // zeros
        REQUIRE(result.data.big.digits[8] == 0xFFFF'FFFEUL);
        // all ff
        REQUIRE(result.data.big.digits[13] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.big.digits[14] == 0xFFFF'FFFFUL);
        REQUIRE(result.data.big.digits[15] == 0xFFFF'FFFFUL);
    }
}

TEST_CASE("Num - divide", "[Num]")
{
    Num ten_e3 = Num(1000);
    Num ten_e6 = ten_e3 * ten_e3;
    Num ten_e9 = ten_e6 * ten_e3;
    Num ten_e12 = ten_e6 * ten_e6;
    Num ten_e18 = ten_e9 * ten_e9;
    Num ten_e24 = ten_e12 * ten_e12;
    Num ten_e36 = ten_e18 * ten_e18;
    Num ten_e48 = ten_e24 * ten_e24;
    Num ten_e72 = ten_e36 * ten_e36;

    SECTION(" Num - check dividends")
    {
        char numbuf[256];
        int L = ten_e48.to_cstring(numbuf, 256);
        REQUIRE(L <= 256);
        REQUIRE(L == 50);
        REQUIRE(0 == strcmp(numbuf, "1000000000000000000000000000000000000000000000000"));
        L = ten_e72.to_cstring(numbuf, 256);
        REQUIRE(L <= 256);
        REQUIRE(L == 74);
        REQUIRE(0 == strcmp(numbuf, "1000000000000000000000000000000000000000000000000000000000000000000000000"));
    }

    SECTION("Num - small divisions")
    {
        Num result = ten_e6 / ten_e3;
        REQUIRE(result == ten_e3);
        result = (ten_e6 + ten_e3 - 1) / ten_e3;
        REQUIRE(result == ten_e3);

        result = ten_e3 / ten_e3;
        REQUIRE(result == 1);
        result = ten_e9 / ten_e3;
        REQUIRE(result == ten_e6);
        result = ten_e3 / ten_e6;
        REQUIRE(result == 0);
    }

    SECTION("Num - large divisions")
    {
        Num result = ten_e36 / ten_e12;
        REQUIRE(result == ten_e24);
        result = ten_e72 / ten_e36;
        REQUIRE(result == ten_e36);

        result = ten_e72 / ten_e48;
        char numbuf[256];
        result.to_cstring(numbuf, 256);
        REQUIRE(result == ten_e24);

        result = ten_e72 / ten_e6;
        result = result / ten_e6;
        result = result / ten_e6;
        result = result / ten_e6;
        REQUIRE(result == ten_e48);
    }
}

TEST_CASE("Num - aliasing", "[Num]")
{
    SECTION("Num - alias add")
    {
        Num a = 5;
        a += a;
        REQUIRE(a == 10);
    }

    SECTION("Num - alias subtract")
    {
        Num a = 5;
        a -= a;
        REQUIRE(a == 0);
    }

    SECTION("Num - alias multiply")
    {
        Num a = 5;
        a *= a;
        REQUIRE(a == 25);
    }

    SECTION("Num - alias divide")
    {
        Num a = 5;
        a /= a;
        REQUIRE(a == 1);
    }
}

TEST_CASE("Num - shift", "[Num]")
{
    Num result;

    result = 0;
    result >>= 1;
    REQUIRE(result == 0);

    result = 1;
    result >>= 1;
    REQUIRE(result == 0);

    result = 2;
    result >>= 1;
    REQUIRE(result == 1);

    result = 3;
    result >>= 1;
    REQUIRE(result == 1);

    result = 0x0FFF'FFFF'FFFF'FFFFLL;
    result >>= 8;
    REQUIRE(result == 0xF'FFFF'FFFF'FFFFLL);

    result = 0x0FFF'FFFF'FFFF'FFFFLL;
    result >>= 33;
    REQUIRE(result == 0x7FF'FFFFL);
}

TEST_CASE("Num - exponentiation", "[Num]")
{
    Num ten = 10;
    Num two = 2;

    Num ten_e2 = ten^two;
    REQUIRE(ten_e2 == 100);

    Num huge = Num(2)^(Num(3)^Num(4));
    char buf[1024];
    int N = huge.to_cstring(buf, 1024);
    REQUIRE(N <= 1024);
    REQUIRE(N == 26);
    REQUIRE(buf == std::string("2417851639229258349412352"));
    //printf(buf); printf("\n");

    Num googol = Num(10)^Num(100);
    REQUIRE(googol.data.len == 11);
    N = googol.to_cstring(buf, 1024);
    REQUIRE(N <= 1024);
    REQUIRE(N == 102);
    REQUIRE(buf == std::string("10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"));
}

TEST_CASE("Num - Mersenne primes", "[Num]")
{
    // start out with 2^0
    int p = 0;
    Num twop = 1;

    std::time_t start = std::time(nullptr);

    int mersenne_factors[] = {
        2, 3, 5, 7, 13,
        17, 19, 31, 61, 89,
        107, 127, 521, 607, 1279,
        2203, 2281, 3217, 4253, 4423,
        9689, 9941, 11213, 19937, 21701,
         23'209,      44'497,    86'243,    110'503,    132'049,
        216'091,     756'839,   859'433,  1'257'787,  1'398'269,
        2'976'221, 3'021'377, 6'972'593, 13'466'917, 20'996'011,
        0
    };

    const int BN = 23'000'000;
    char* buf = new char[BN];
    for (int i = 0; mersenne_factors[i] != 0; i++)
    {
        int nextpow = mersenne_factors[i];
        twop *= Num(2) ^ Num(nextpow - p);
        p = mersenne_factors[i];

        Num mp = twop - Num(1);
        int N = mp.to_cstring(buf, BN);
        REQUIRE(N <= BN);

        std::time_t now = std::time(nullptr);
        std::cout << "t+" << now-start << ":  d=" << mp.data.len << "  l=" << N-1;
        std::cout << "  n=" << i+1 << "  p=" << p;
        if (strlen(buf) < 2*12+2)
            std::cout << "  Mp=" << buf << std::endl;
        else
        {
            char prefix[13]; char suffix[13];
            memcpy(prefix, buf, 12); prefix[12] = 0;
            memcpy(suffix, buf+strlen(buf)-12, 12); suffix[12] = 0;
            std::cout << "  Mp=" << prefix << "..." << suffix << std::endl;
        }
    }

    delete[] buf;

    //Num mersenne74 = Num(2)^Num(74'207'281);
    //REQUIRE(mersenne74.data.len == 500);
}

TEST_CASE("Num - very large values", "[Num]")
{
    Num huge1 = Num(2)^(Num(3)^Num(4));
    Num huge2 = Num(2)^(Num(3)^Num(5));
    Num huge3 = Num(3)^(Num(4)^Num(5));

    Num huge4 = huge1 * huge2 * huge3;
    huge4 /= huge1;
    REQUIRE(huge4 == huge2*huge3);
    huge4 /= huge2;
    REQUIRE(huge4 == huge3);

    huge4 = huge1 * huge2 * huge3;
    huge4 /= huge2;
    REQUIRE(huge4 == huge1*huge3);
    huge4 /= huge1;
    REQUIRE(huge4 == huge3);
}

TEST_CASE("Num - conversions", "[Num]")
{
    SECTION("string_view to number")
    {
        Num result;
        result.from_string(std::string_view("12345678"));
        REQUIRE(result == 12345678LL);
        result.from_string(std::string_view("FFFFFFFF"), 16);
        REQUIRE(result == (65536LL*65536)-1);
        result.from_string(std::string_view("377"), 8);
        REQUIRE(result == 255);
        result.from_string(std::string_view("F0F0'F0F0'F0F0'F0F0'0F0F'0F0F'0F0F'0F0F"), 16);
        REQUIRE_FALSE(result.data.nonlocal);
        REQUIRE(result.data.len == 4);
        REQUIRE(result.data.buf[0] == 0x0F0F'0F0FL);
        REQUIRE(result.data.buf[1] == 0x0F0F'0F0FL);
        REQUIRE(result.data.buf[2] == 0xF0F0'F0F0L);
        REQUIRE(result.data.buf[3] == 0xF0F0'F0F0L);
    }

    SECTION("Num to string")
    {
        char numbuf[256];
        Num result = 12345678;
        int L = result.to_cstring(numbuf, 256, 10);
        REQUIRE(L < 256);
        REQUIRE(L == 9); // includes zero terminator
        REQUIRE(numbuf == std::string("12345678"));

        result = Num(0xFFFFFFFF) * Num(0xFFFFFFFF);
        L = result.to_cstring(numbuf, 256, 16);
        REQUIRE(numbuf == std::string("FFFFFFFE00000001"));
    }
}

#if 0

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

    // shouldn't compile yet
    multiplicand = 1;
    result = multiplicand * 0x7FFF'FFFF'FFFF'FFFFLL;

    // Test multiply and divide
    Num factors[10];
    factors[0] = 65537;
    REQUIRE(factors[0].data.len == 1);
    REQUIRE(factors[0].small.buf[0] == 65537);

    for (int i = 1; i < 10; i++)
    {
        factors[i] = factors[i-1] * factors[i-1];
        REQUIRE(factors[i].databuffer()[factors[i].data.len-1] != 0);
    }

    Num results[10];
    for (int i = 1; i < 10; i++)
    {
        results[i-1] = factors[i] / factors[i-1];
        REQUIRE(factors[i-1] == results[i-1]);
    }
}

TEST_CASE("comparisons", "[Num]")
{
    Num lhs(1);
    Num rhs(1);
    REQUIRE(lhs == rhs);

    rhs = 2;
    REQUIRE(lhs != rhs);
}
// "0FFFFFFF'FFFFFFFF'FFFFFFFF'FFFFFFFF'FFFFFFFF'FFFFFFFF'FFFFFFFF'FFFFFFFF
TEST_CASE("string to Num", "[Num]")
{
    std::string buf;
    buf.reserve(32);

    buf = "1";
    Num n(1);
    Num c;
    for (int i = 0; i < 100; i++)
    {
        n *= 10;
        buf += "0";
        c.from_string(std::string_view(buf));

        REQUIRE(n == c);
    }

    buf = "F";
    n = 15;
    for (int i = 0; i < 100; i++)
    {
        n = (n * 16) + 15;
        buf += "F";
        REQUIRE(int(buf.length()) == i + 2);
        c.from_string(std::string_view(buf), 16);

        REQUIRE(n == c);
    }
}

#endif

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
#endif

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

#if 0
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
