// buffer1.cpp
// - trivial buffer

#include "buffer1.h"

#include <cstring>
#include <iostream>

void StringSearch1(Buffer1<char>& buf, char* str)
{
    int n = strlen(str);
    while (buf.cursor < buf.end - n)
    {
        if (0 == strncmp(buf.cursor, str, n))
            std::cout << "found at " << buf.cursor - buf.buf << "\n";

        buf.cursor++; // naive string search
    }
}
