// buffer3.h

#pragma once

#include <stdint.h>
#include <cstdio>

template <typename T>
class Buffer3
{
public:
    Buffer3() : buf(nullptr), cursor(nullptr), end(nullptr), bufEnd(nullptr),
                fp(nullptr), viewPos(-1) {}

    void open(char* path);
    void close();
    bool seek(int64_t pos);
    bool preflight(int amt);

    T* start; // pointer to beginning of active buffer
    T* cursor; // pointer to cursor inside buffer
    T* end; // pointer to end of data in buffer
    T* bufEnd; // pointer to end of buffer

    FILE* fp; // the file we are filling the buffer from
    int64_t viewPos; // offset in file matching data in buf
};
