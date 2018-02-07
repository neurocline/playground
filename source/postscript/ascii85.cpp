// ascii85.cpp
// - convert data to/from ascii85
// - see https://en.wikipedia.org/wiki/Ascii85

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "ascii85.h"

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

void ToAscii85(Ascii85Buf& buf)
{
    int len = buf.dataLen;
    uint8_t* data = buf.data;

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

        // If it's zero, we just output a single char
        if (v == 0)
            buf.putA85('z');

        // Otherwise, do the 4to5 encoding and write it
        else
        {
            buf.reserveA85(5);
            QuadTo85(v, buf.a85 + buf.a85Len);
            buf.a85Len += 5;
        }
    }

    // If there are octets left, pad before converting. This cannot use zero-encoding,
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
            buf.putA85(tbuf[5-i]);
    }
}

void FromAscii85(Ascii85Buf& buf)
{
    int len = buf.a85Len;
    char* a85 = buf.a85;

    while (len > 0)
    {
        // A 'z' character expands to 4 zero bytes of output
        if (*a85 == 'z')
        {
            buf.reserveData(4);
            *(uint32_t*)(buf.data + buf.dataLen) = 0;
            buf.dataLen += 4;
            len--;
            a85++;
        }

        // If we have 5 a85 chars, then decode to 4 bytes of output
        // and write as big-endian integer
        else if (len >= 5)
        {
            uint32_t v = A85ToQuad(a85);
            buf.reserveData(4);
            buf.data[buf.dataLen + 0] = (v >> 24);
            buf.data[buf.dataLen + 1] = ((v >> 16) & 0xFF);
            buf.data[buf.dataLen + 2] = ((v >> 8) & 0xFF);
            buf.data[buf.dataLen + 3] = (v & 0xFF);
            buf.dataLen += 4;
            len -= 5;
            a85 += 5;
        }

        // If we don't have at least 5 bytes of data, then do a
        // partial decode with what we have
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
            buf.reserveData(len);
            buf.data[buf.dataLen++] = (v >> 24);
            if (len > 1)
            {
                buf.data[buf.dataLen++] = ((v >> 16) & 0xFF);
                if (len > 2)
                    buf.data[buf.dataLen++] = ((v >> 8) & 0xFF);
            }

            len = 0; // we are done
        }
    }
}

#include "../catch.hpp"

TEST_CASE("Simple ToAscii85", "[ascii85]")
{
    Ascii85Buf to85; to85.reserveA85(256);
    Ascii85Buf from85; from85.reserveData(256);

    // Zero-length data should be zero-length a85, and
    // zero-length a85 should be zero-length data
    to85.data = (uint8_t*)"";
    to85.dataLen = 0;
    to85.a85Len = 0;
    ToAscii85(to85);
    REQUIRE(to85.a85Len == 0);

    from85.a85 = to85.a85;
    from85.a85Len = to85.a85Len;
    from85.dataLen = 0;
    FromAscii85(from85);
    REQUIRE(from85.dataLen == to85.dataLen);

    // Four zeros turn to a single "z", and a single "z"
    // turns back into four zeros
    to85.data = (uint8_t*)("\0\0\0\0");
    to85.dataLen = 4;
    to85.a85Len = 0;
    ToAscii85(to85);
    REQUIRE(to85.a85Len == 1);
    REQUIRE(0 == strncmp(to85.a85, "z", 1));

    from85.a85 = to85.a85;
    from85.a85Len = to85.a85Len;
    from85.dataLen = 0;
    FromAscii85(from85);
    REQUIRE(from85.dataLen == to85.dataLen);
    REQUIRE(0 == memcmp(from85.data, to85.data, to85.dataLen));

    // Single bytes turn to two characters in the output
    // and vice-versa
    to85.data = (uint8_t*)(".");
    to85.dataLen = 1;
    to85.a85Len = 0;
    ToAscii85(to85);
    REQUIRE(to85.a85Len == 2);
    REQUIRE(0 == strncmp(to85.a85, "/c", 2));

    from85.a85 = to85.a85;
    from85.a85Len = to85.a85Len;
    from85.dataLen = 0;
    FromAscii85(from85);
    REQUIRE(from85.dataLen == to85.dataLen);
    REQUIRE(0 == memcmp(from85.data, to85.data, to85.dataLen));

    free(to85.a85);
    free(from85.data);
}

// The missing test - exercise 0 to 4 byte conversions, since everything is
// in chunks of 4 bytes (to85) or 5 bytes (from85). This can probably be done
// analytically, which would be a good thing, rather than a bunch of magic
// values. Still have the long magic values, of course, as a check for buffer
// handling.

// This kind of test requires a ton of handwork or a reference implementation to
// generate the data.
TEST_CASE("ToAscii85", "[ascii85]")
{
    char* rawdata =
        R"(Man is distinguished, not only by his reason, but by this singular passion from other animals, )"
        R"(which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable )"
        R"(generation of knowledge, exceeds the short vehemence of any carnal pleasure.)";

    char* a85data =
        R"ascii85(9jqo^BlbD-BleB1DJ+*+F(f,q/0JhKF<GL>Cj@.4Gp$d7F!,L7@<6@)/0JDEF<G%<+EV:2F!,)ascii85"
        R"ascii85(O<DJ+*.@<*K0@<6L(Df-\0Ec5e;DffZ(EZee.Bl.9pF"AGXBPCsi+DGm>@3BB/F*&OCAfu2/AKY)ascii85"
        R"ascii85(i(DIb:@FD,*)+C]U=@3BN#EcYf8ATD3s@q?d$AftVqCh[NqF<G:8+EV:.+Cf>-FD5W8ARlolDIa)ascii85"
        R"ascii85(l(DId<j@<?3r@:F%a+D58'ATD4$Bl@l3De:,-DJs`8ARoFb/0JMK@qB4^F!,R<AKZ&-DfTqBG%G)ascii85"
        R"ascii85(>uD.RTpAKYo'+CT/5+Cei#DII?(E,9)oF*2M7/c)ascii85";

    Ascii85Buf to85; to85.reserveA85(256);
    to85.data = (uint8_t*) rawdata;
    to85.dataLen = strlen(rawdata);
    to85.a85Len = 0;

    Ascii85Buf from85; from85.reserveData(256);
    from85.a85 = a85data;
    from85.a85Len = strlen(a85data);
    from85.dataLen = 0;

    ToAscii85(to85);
    REQUIRE(0 == strncmp(to85.a85, a85data, from85.a85Len));

    FromAscii85(from85);
    REQUIRE(0 == strncmp((char*)from85.data, rawdata, to85.dataLen));
}
