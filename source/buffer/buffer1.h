// buffer1.h
// - trivial buffer

#pragma once

template <typename T>
class Buffer1
{
public:
    Buffer1() : buf(nullptr), cursor(nullptr), end(nullptr) {}

    T* buf; // pointer to beginning of active buffer
    T* cursor; // pointer to cursor inside buffer
    T* end; // pointer to end of buffer
};
