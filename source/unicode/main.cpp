// main.cpp

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "../catch.hpp"

#include <assert.h>

int ConvertUTF8toUTF16(uint8_t* utf8, uint16_t* utf16);
int ConvertUTF16toUTF8(uint16_t* utf16, uint8_t* utf8);

TEST_CASE("UTF-8 roundtrip", "[Unicode]")
{
    const int utf1byte = 1 << 7;
    const int utf2byte = 1 << 11;
    const int utf3byte = 1 << 16;
    const int utf4byte = 1 << 21;

    for (int ch = 0; ch < 1'112'064; ++ch)
    {
        // characters in the range 0xD800..0xDBFF are disallowed (this is the high
        // surrogate area, used for surrogate characters in UTF-16)
        if (ch >= 0xD800 && ch <= 0xDFFF)
            continue;

        // Generate a zero-terminated string of 1 UTF-8 character
        // (up to 5 bytes)
        uint8_t utf8[5]; memset(utf8, 0, sizeof(utf8));
        int len = 0;

        // 0xxxxxxx
        if (ch < utf1byte)
        {
            len = 1;
            utf8[0] = (uint8_t) ch;
        }

        // 110xxxxx 10xxxxxx
        else if (ch < utf2byte)
        {
            utf8[0] = (uint8_t) (0b1100'0000 | (ch >> 6));
            utf8[1] = (uint8_t) (0b1000'0000 | (ch & 0x3F));
        }

        // 1110xxxx 10xxxxxx 10xxxxxx
        else if (ch < utf3byte)
        {
            utf8[0] = (uint8_t) (0b1110'0000 | (ch >> 12));
            utf8[1] = (uint8_t) (0b1000'0000 | ((ch >> 6) & 0x3F));
            utf8[2] = (uint8_t) (0b1000'0000 | (ch & 0x3F));
        }

        else if (ch < utf4byte)
        {
            utf8[0] = (uint8_t) (0b1111'0000 | (ch >> 18));
            utf8[1] = (uint8_t) (0b1000'0000 | ((ch >> 12) & 0x3F));
            utf8[2] = (uint8_t) (0b1000'0000 | ((ch >> 6) & 0x3F));
            utf8[3] = (uint8_t) (0b1000'0000 | (ch & 0x3F));
        }

        else
            assert(!"ran off end of UTF-8 range");

        uint16_t utf16[3]; memset(utf16, 0, sizeof(utf16));
        ConvertUTF8toUTF16(utf8, utf16);

        uint8_t utf8out[5]; memset(utf8out, 0, sizeof(utf8out));
        ConvertUTF16toUTF8(utf16, utf8out);
        REQUIRE(0 == memcmp(utf8, utf8out, sizeof(utf8)));
    }
}

int ConvertUTF8toUTF16(uint8_t* utf8, uint16_t* utf16)
{
    auto res = MultiByteToWideChar(
        CP_UTF8,
        0,
        (LPCSTR) utf8,
        -1,
        (LPWSTR) utf16,
        65536); // hope that we don't overflow!
    return res;
}

int ConvertUTF16toUTF8(uint16_t* utf16, uint8_t* utf8)
{
    auto res = WideCharToMultiByte(
        CP_UTF8,
        0,
        (LPWSTR) utf16,
        -1,
        (LPSTR) utf8,
        65536,
        NULL,
        NULL);
    return res;
}
