// buffer2.h

#pragma once

#include <stdint.h>
#include <cstdio>

template <typename T>
class Buffer2
{
public:
    Buffer2() : buf(nullptr), cursor(nullptr), end(nullptr), bufEnd(nullptr),
                fp(nullptr), viewPos(-1) {}

    void open(char* path);
    void close();
    bool seek(int64_t pos);
    bool preflight(int amt);

    T* buf; // pointer to beginning of active buffer
    T* cursor; // pointer to cursor inside buffer
    T* end; // pointer to end of data in buffer
    T* bufEnd; // pointer to end of buffer

    FILE* fp; // the file we are filling the buffer from
    int64_t viewPos; // offset in file matching data in buf
};

template <typename T>
class Buffer2b
{
public:
    Buffer2b() : buf(nullptr), cursor(nullptr), end(nullptr), fileStart(-1) {}

    bool seek(int64_t /*pos*/) { return false; }
    bool prefetch(int /*amt*/) { return false; }

    T* buf; // pointer to beginning of active buffer
    T* cursor; // pointer to cursor inside buffer
    T* end; // pointer to end of buffer

    bool dirty; // true if data in buf has been modified
    operator bool() { return cursor < end; }

    int64_t viewPos; // offset in file matching data in buf
};
