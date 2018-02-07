// ascii85.cpp
// - convert data to/from ascii85
// - see https://en.wikipedia.org/wiki/Ascii85

#include <cstdint>
#include <cstdlib>
#include <cstring>

class streambuf
{
public:
    streambuf() : len(0), size(0), buf(nullptr) {}
    void reserve(int n);

    inline void put(char c)
    {
        reserve(1);
        buf[len++] = c;
    }

    char* buf;
    int len;
    int size;
};

void streambuf::reserve(int n)
{
    if (len + n <= size)
        return;

    // find the next doubling, starting with 256
    int nsize = size ? 2*size : 256;
    while (nsize < len + n)
        nsize *= 2;

    char* nbuf = (char*) malloc(nsize);
    if (len != 0)
    {
        memcpy(nbuf, buf, len);
        free(buf);
    }
    buf = nbuf;
    size = nsize;
}

// Convert a single 4-byte value into a 5-character string
// (p points to at least 5 allocated bytes)
inline void QuadTo85(uint32_t v, char* a85)
{
    for (int i = 4; i >= 0; i--)
    {
        char rem = v % 85;
        v = v / 85;
        a85[i] = '!' + rem;
    }
}

// Convert a 5-character ascii85 string to a 4-byte value
// (a85 points to 5 valid ascii85 bytes)
uint32_t A85ToQuad(char* a85)
{
    uint32_t v = 0;
    for (int i = 0; i < 5; i++)
        v = (v * 85) + (*a85++ - '!');
    return v;
}

char* ToAscii85(uint8_t* data, int len)
{
    streambuf o;

    // Convert groups of 4 octects - end is handled specially
    // Note that if we really cared about speed, this is completely parallelizable,
    // each quad can be converted independently of other quads; not multithreading,
    // but instruction-level parallelism, or maybe using x86 SSE instructions.
    while (len >= 4)
    {
        // get a big-endian int (we expect compilers to turn this into efficient code)
        uint32_t v = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]);
        len -= 4;
        data += 4;

        if (v == 0)
            o.put('z');
        else
        {
            o.reserve(5);
            QuadTo85(v, o.buf + o.len);
            o.len += 5;
        }
    }

    // If there are octects left, pad before converting. This cannot use zero-encoding,
    // because we would then lose the idea of final length.
    if (len != 0)
    {
        int pad = 4 - len;
        uint32_t v = 0;
        for (; len != 0; len--)
            v = (v << 8) | *data++;
        for (int i = 0; i < pad; i++)
            v = v << 8;

        // When outputting, omit as many output bytes as we padded with. We will output
        // 2 to 4 chars (min output is 2 chars for 1 input byte).
        char tbuf[5];
        QuadTo85(v, tbuf);
        for (int i = 5; i > pad; i--)
            o.put(tbuf[5-i]);
    }

    // zero-terminate
    o.put(0);
    return o.buf;
}

streambuf FromAscii85(char* a85, int len = -1)
{
    streambuf o;
    if (len < 0)
        len = strlen(a85);

    while (len > 0)
    {
        if (*a85 == 'z')
        {
            o.reserve(4);
            *(uint32_t*)(o.buf+o.len) = 0;
            o.len += 4;
            len--;
            a85++;
        }
        else if (len >= 5)
        {
            uint32_t v = A85ToQuad(a85);
            o.reserve(4);
            o.buf[o.len++] = (v >> 24);
            o.buf[o.len++] = ((v >> 16) & 0xFF);
            o.buf[o.len++] = ((v >> 8) & 0xFF);
            o.buf[o.len++] = (v & 0xFF);
            len -= 5;
            a85 += 5;
        }
        else
        {
            char tbuf[5];
            int i = 0;
            for (; i < len; i++)
                tbuf[i] = *a85++;
            for (; i < 5; i++)
                tbuf[i] = 'u';

            uint32_t v = A85ToQuad(tbuf);

            // Output, skipping as many bytes of output as we had to pad
            // by; this works out for 5to4
            len -= 1;
            o.reserve(len);
            o.buf[o.len++] = (v >> 24);
            if (len > 1)
                o.buf[o.len++] = ((v >> 16) & 0xFF);
            if (len > 2)
                o.buf[o.len++] = ((v >> 8) & 0xFF);

            len = 0;
        }
    }

    return o;
}

#include "../catch.hpp"

TEST_CASE("Simple ToAscii85", "[ascii85]")
{
    char* a85;

    // A zero-length input turns to zero-length output
    a85 = ToAscii85((uint8_t*)(""), 0);
    REQUIRE(0 == strcmp(a85, ""));
    free(a85);

    // Four zeros turn to a single "z"
    a85 = ToAscii85((uint8_t*)("\0\0\0\0"), 4);
    REQUIRE(0 == strcmp(a85, "z"));
    free(a85);

    // Single bytes turn to two characters in the output
    a85 = ToAscii85((uint8_t*)("."), 1);
    REQUIRE(0 == strcmp(a85, "/c"));
    free(a85);

    // Round trip
    a85 = ToAscii85((uint8_t*)(""), 0);
    streambuf o = FromAscii85(a85);
    REQUIRE(o.len == 0);
    free(a85);
    free(o.buf);

    a85 = ToAscii85((uint8_t*)("\0\0\0\0"), 4);
    o = FromAscii85(a85);
    REQUIRE(o.len == 4);
    REQUIRE(0 == strncmp(o.buf, "\0\0\0\0", 4));
    free(a85);
    free(o.buf);

    a85 = ToAscii85((uint8_t*)("."), 1);
    o = FromAscii85(a85);
    REQUIRE(o.len == 1);
    REQUIRE(0 == strncmp(o.buf, ".", 1));
    free(a85);
    free(o.buf);
}

TEST_CASE("ToAscii85", "[ascii85]")
{
    char* input =
        R"(Man is distinguished, not only by his reason, but by this singular passion from other animals, )"
        R"(which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable )"
        R"(generation of knowledge, exceeds the short vehemence of any carnal pleasure.)";

    char* expected =
        R"ascii85(9jqo^BlbD-BleB1DJ+*+F(f,q/0JhKF<GL>Cj@.4Gp$d7F!,L7@<6@)/0JDEF<G%<+EV:2F!,)ascii85"
        R"ascii85(O<DJ+*.@<*K0@<6L(Df-\0Ec5e;DffZ(EZee.Bl.9pF"AGXBPCsi+DGm>@3BB/F*&OCAfu2/AKY)ascii85"
        R"ascii85(i(DIb:@FD,*)+C]U=@3BN#EcYf8ATD3s@q?d$AftVqCh[NqF<G:8+EV:.+Cf>-FD5W8ARlolDIa)ascii85"
        R"ascii85(l(DId<j@<?3r@:F%a+D58'ATD4$Bl@l3De:,-DJs`8ARoFb/0JMK@qB4^F!,R<AKZ&-DfTqBG%G)ascii85"
        R"ascii85(>uD.RTpAKYo'+CT/5+Cei#DII?(E,9)oF*2M7/c)ascii85";

    char* a85 = ToAscii85((uint8_t*)input, strlen(input));
    REQUIRE(0 == strcmp(a85, expected));
    streambuf o = FromAscii85(a85);
    REQUIRE(0 == strncmp(o.buf, input, strlen(input)));
    free(a85);
    free(o.buf);
}

TEST_CASE("raw string", "[ascii85]")
{
    char* raw = R"(a)"
                R"(b)"
                R"(c)";
    REQUIRE(0 == memcmp(raw, "abc", 3));
}
