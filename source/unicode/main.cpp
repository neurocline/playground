// main.cpp

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "../catch.hpp"

#include <assert.h>
#include <iostream>

int ConvertUTF8toUTF16(uint8_t* utf8, uint16_t* utf16);
int ConvertUTF16toUTF8(uint16_t* utf16, uint8_t* utf8);
bool IsUTF8Char(uint8_t* utf8);

#if 0
TEST_CASE("UTF-8 roundtrip", "[Unicode]")
{
    const int utf1byte = 1 << 7;
    const int utf2byte = 1 << 11;
    const int utf3byte = 1 << 16;
    const int utf4byte = 1 << 21;
    // UTF-8 is designed for up to 6 byte sequences, but Unicode currently
    // only goes to 0x0010FFFF, which fits in 4 UTF-8 bytes)

    for (int ch = 0; ch < 1'112'064; ++ch)
    {
        // characters in the range 0xD800..0xDBFF are disallowed (this is the high
        // surrogate area, used for surrogate characters in UTF-16)
        if (ch >= 0xD800 && ch <= 0xDFFF)
            continue;

        // Even though 0xFDD0..0xFDEF, 0xFFFE, and 0xFFFF aren't legal UTF-16
        // characters, we still generate and round-trip them. They are values that
        // a Unicode program can see, they just aren't legal to put into
        // serialized Unicode text.

        // The other magic character is 0xFFFD, which is the Replacement Character.
        // This should normally never been seen - if it is seen, something has gone
        // wrong in an encoding step.

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
#endif

void TrackRange(bool& wasValid, bool isValid, uint32_t& start, uint32_t pos, int& count)
{
    if (isValid != wasValid)
    {
        if (pos != start)
        {
            if (wasValid && !isValid)
                printf("%02X-%02X valid\n", start, pos-1);
            else if (!wasValid && isValid)
                printf("%02X-%02X invalid\n", start, pos-1);
        }
        start = pos;
    }
    wasValid = isValid;
    if (isValid)
        count += 1;
}

TEST_CASE("Illegal UTF-8 1-byte sequences", "[Unicode]")
{
    int count = 0;
    std::cout << "1-byte UTF-8 sequences\n";
    bool validUTF8 = false;
    uint32_t start = 0;
    for (uint32_t i = 0; i < 0xC0; ++i)
    {
        uint8_t utf8[5]; memset(utf8, 0, sizeof(utf8));
        int len = 0;

        utf8[0] = (uint8_t) i;
        len = 1;

        bool valid = IsUTF8Char(utf8);
        TrackRange(validUTF8, valid, start, i, count);
    }

    TrackRange(validUTF8, !validUTF8, start, 0x100, count);
    std::cout << count-1 << "/" << 0xC0 << " valid\n";
}

TEST_CASE("Illegal UTF-8 2-byte sequences", "[Unicode]")
{
    int count = 0;
    std::cout << "2-byte UTF-8 sequences\n";
    std::cout << "0000-7FFF skipping\n";

    bool validUTF8 = false;
    uint32_t start = 0x0'C000;
    for (uint32_t i = 0x0'C000; i < 0x0'E0'00; ++i)
    {
        uint8_t utf8[5]; memset(utf8, 0, sizeof(utf8));
        int len = 0;

        utf8[0] = (uint8_t) (i >> 8);
        utf8[1] = (uint8_t) (i & 0xFF);
        len = 2;

        bool valid = IsUTF8Char(utf8);
        TrackRange(validUTF8, valid, start, i, count);
    }

    TrackRange(validUTF8, !validUTF8, start, 0x1'0000, count);
    std::cout << count-1 << "/" << 0xE000-0xC000 << " valid\n";
}

TEST_CASE("Illegal UTF-8 3-byte sequences", "[Unicode]")
{
    int count = 0;
    std::cout << "3-byte UTF-8 sequences\n";
    std::cout << "000000-DFFFFF skipping\n";

    bool validUTF8 = false;
    uint32_t start = 0x0'E0'00'00;
    for (uint32_t i = 0x0'E0'00'00; i < 0x0'F0'00'00; ++i)
    {
        uint8_t utf8[5]; memset(utf8, 0, sizeof(utf8));
        int len = 0;

        utf8[0] = (uint8_t) (i >> 16);
        utf8[1] = (uint8_t) ((i >> 8) & 0xFF);
        utf8[2] = (uint8_t) (i & 0xFF);
        len = 3;

        bool valid = IsUTF8Char(utf8);
        TrackRange(validUTF8, valid, start, i, count);
    }

    TrackRange(validUTF8, !validUTF8, start, 0x1'00'00'00, count);
    std::cout << count-1 << "/" << 0xF00000-0xE00000 << " valid\n";
}

TEST_CASE("Illegal UTF-8 4-byte sequences", "[Unicode]")
{
    int count = 0;
    std::cout << "4-byte UTF-8 sequences\n";
    std::cout << "00000000-EFFFFFFF skipping\n";

    bool validUTF8 = false;
    uint32_t start = 0x0'F0'00'00'00;
    for (uint32_t i = 0x0'F0'00'00'00; i < 0xFF'FF'FF'FF; ++i)
    {
        uint8_t utf8[5]; memset(utf8, 0, sizeof(utf8));
        int len = 0;

        utf8[0] = (uint8_t) (i >> 24);
        utf8[1] = (uint8_t) ((i >> 16) & 0xFF);
        utf8[2] = (uint8_t) ((i >> 8) & 0xFF);
        utf8[3] = (uint8_t) (i & 0xFF);
        len = 4;

        bool valid = IsUTF8Char(utf8);
        TrackRange(validUTF8, valid, start, i, count);
    }

    TrackRange(validUTF8, !validUTF8, start, 0x00'00'00'00, count);
    std::cout << count-1 << "/" << 0x10000000 << " valid\n";
}

// This is designed to do early-out as fast as possible. Most characters are short,
// so we check short sequences first.
// Don't we really want UTF8CharLen instead, and return 0 for bad character?
bool IsUTF8Char(uint8_t* utf8)
{
    // ------- Check 1-byte characters -------
    // First byte is 0xxxxxxx
    if (utf8[0] < (1<<7))
        return true;

    // ------- Check 2-byte characters -------
    // First byte is 110xxxxx, so anything less is illegal
    else if (utf8[0] < 0xC0)
        return false;

    // Second byte must be 10xxxxxx
    else if ((utf8[1] & 0xC0) != 0x80)
        return false;

    // Decode the value. It must be in the range 0x80..0x7FF
    else if ((utf8[0] & 0xE0) == 0xC0)
    {
        int v1 = utf8[0] & 0x1f;
        int v2 = utf8[1] & 0x3F;
        int v = (v1 << 6) | v2;
        if (v < 0x80 || v > 0x7FF)
            return false;
        return true;
    }

    // ------- Check 3-byte characters -------
    // First byte is 1110xxx, so anything less is illegal
    else if (utf8[0] < 0xE0)
        return false;

    // Third byte must be 10xxxxxx
    else if ((utf8[2] & 0xC0) != 0x80)
        return false;

    // Decode the value. It must be in the range 0x800..0xFFFF
    else if ((utf8[0] & 0xF0) == 0xE0)
    {
        int v1 = utf8[0] & 0x0f;
        int v2 = utf8[1] & 0x3F;
        int v3 = utf8[2] & 0x3F;
        int v = (v1 << 12) | (v2 << 6) | v3;
        if (v < 0x800 || v > 0xFFFF)
            return false;
        return true;
    }

    // ------- Check 4-byte characters -------
    // First byte is 11110xx, so anything less is illegal
    else if (utf8[0] < 0xF0)
        return false;

    // Fourth byte must be 10xxxxxx
    else if ((utf8[3] & 0xC0) != 0x80)
        return false;

    // Decode the value. It must be in the range 0x800..0xFFFF
    else if ((utf8[0] & 0xF8) == 0xF0)
    {
        int v1 = utf8[0] & 0x0f;
        int v2 = utf8[1] & 0x3F;
        int v3 = utf8[2] & 0x3F;
        int v4 = utf8[3] & 0x3F;
        int v = (v1 << 18) | (v2 << 12) | (v3 << 6) | v4;
        if (v < 0x10000 || v > 0x1FFFFF)
            return false;
        return true;
    }

    return false;
}

#ifdef _WIN32
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
#endif
