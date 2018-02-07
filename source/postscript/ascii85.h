// ascii85.h
// - convert data to/from ascii85
// - see https://en.wikipedia.org/wiki/Ascii85

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>

// This is a very simple transport for data to/from the ascii85
// conversion routines. We could make the caller allocate buffers,
// but that's only possible if we aren't streaming data, and we should
// be designing routines to stream where possible, because that's how
// Postscript expects to operate. That said, (1) our routines don't
// stream at the moment, and (2) this is probably an obsolete requirement
// given size of even the smallest computer nowadays.
//
// Note that we can either have data=input and a85=output, or
// data=output and a85=input.
//
// The intention is that input can be a readstream and output can be a
// writestream.

class Ascii85Buf
{
public:
    Ascii85Buf() : data(nullptr), dataLen(0), dataSize(0),
                   a85(nullptr), a85Len(0), a85Size(0)
    {
        ialloc = internal_alloc;
        ifree = internal_free;
    }

    // binary data
    //void reserveData(int n);
    //inline void putData(uint8_t b);

    // ascii85-encoded data
    //void reserveA85(int n);
    //inline void putA85(char c);

    // ------------------------------------------------------

    // Grow data if needed (this should inline down to just a call to reserve)
    inline void reserveData(int n)
    {
        if (dataLen + n < dataSize)
            return;

        void* p = (void*)data;
        reserve(p, n, dataLen, dataSize);
        data = (uint8_t*) p;
    }

    inline void putData(uint8_t b)
    {
        reserveData(1);
        data[dataLen++] = b;
    }

    // ascii85-encoded data
    inline void reserveA85(int n)
    {
        if (a85Len + n < a85Size)
            return;

        void* p = (void*)a85;
        reserve(p, n, a85Len, a85Size);
        a85 = (char*) p;
    }

    inline void putA85(char c)
    {
        reserveA85(1);
        a85[a85Len++] = c;
    }

    // Grow data in some buffer as needed (we can assume callers will usually
    // pass preallocataed buffers of sufficient size, this is for edge cases)
    inline void reserve(void*& buf, int n, int& len, int& size)
    {
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

    void* (*ialloc)(int size);
    void (*ifree)(void* p);

    static void* internal_alloc(int size) { return malloc(size); }
    static void internal_free(void* p) { return free(p); }

    // ------------------------------------------------------

    // binary data
    uint8_t* data; // pointer to binary data
    int dataLen; // current bytes in data
    int dataSize; // max size of currently allocated data

    // ascii85-encoded data
    char* a85; // pointer to a85-encodede data
    int a85Len; // length of data in a85
    int a85Size; // max size of currently allocated a85
};
