// buffer3.cpp
// - buffer with intelligent seek and preflight

#include "buffer3.h"

#include <algorithm>
#include <cstring>
#include <iostream>

void BoyerMooreStringSearch(Buffer3<char>& buf, char* str)
{
    int patlen = strlen(str);

    // Generate the simpler of the Boyer-Moore skip tables; for characters in the pattern, we skip to
    // the relative position in the pattern. We 
    int text_skips[256];

    // - For each character not in the pattern, we skip the full pattern length
    for (int i = 0; i < 256; i++)
        text_skips[i] = patlen;

    // - For each character in the pattern, we skip to the relative position
    //   of that character in the pattern (we index backwards through the pattern
    //   so the earliest position of a repeated character has priority)
    for (int i = patlen - 1; i >= 0; --i)
        text_skips[str[i]] = i;

    for (;;)
    {
        int i = patlen - 1;

        if (!buf.preflight(patlen))
            break;

        // compare backwards until we find a mismatch or we've done the
        // whole string
        int j = patlen - 1;
        for (; j >= 0 && buf.cursor[i] == str[j]; --i, --j)
            ;

        // If we found a mismatch, skip to the next possible search location
        if (j >= 0)
        {
            //i += std::max(delta1[buf.cursor[i]], delta2[j]);
            buf.cursor += text_skips[buf.cursor[i]];
            continue;
        }

        // We found the string at buf.cursor
        std::cout << "found at " << buf.viewPos + buf.cursor - buf.start << "\n";

        // Look for next possible match
        buf.cursor += 1;
        continue;
    }
}
