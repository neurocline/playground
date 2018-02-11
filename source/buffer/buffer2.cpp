// buffer2.cpp
// - buffer with seek and preflight

#include "buffer2.h"

#include <cstring>
#include <iostream>

void StringSearch2(Buffer2<char>& buf, char* str)
{
    int n = strlen(str);
    while (buf.cursor < buf.end - n)
    {
        buf.preflight(n);
        if (0 == strncmp(buf.cursor, str, n))
            std::cout << "found at " << buf.viewPos + buf.cursor - buf.buf << "\n";

        buf.cursor++; // naive string search
    }
}

void SearchFile()
{
}

template <typename T>
bool Buffer2<T>::seek(int64_t pos)
{
    if (pos >= viewPos && buf + pos < end)
        cursor = buf + pos - viewPos;
    else
        viewPos = pos;
    return true; // should range-check against file bounds?
}

template <typename T>
bool Buffer2<T>::preflight(int amt)
{
    if (cursor + amt <= end)
        return true;

    auto filePos = viewPos + cursor - buf;
    fseek(fp, filePos, SEEK_SET);
    auto amtRead = fread(buf, 1, end - buf, fp);

    cursor = buf;
    end = buf + amtRead;
    return cursor + amt <= end;
}

template <typename T>
void Buffer2<T>::open(char* path)
{
    const int BUF_SIZE = 1'000'000;
    fp = fopen(path, "rb");
    viewPos = 0;
    buf = (T *) malloc(BUF_SIZE);
    bufEnd = buf + BUF_SIZE;

    cursor = nullptr;
    end = nullptr;
}

template <typename T>
void Buffer2<T>::close()
{
    fclose(fp);
    free(buf);
    buf = nullptr;
    viewPos = -1;
    cursor = nullptr;
    end = nullptr;
}

template class Buffer2<char>;
