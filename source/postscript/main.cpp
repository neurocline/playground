// main.cpp
// - Simple Postscript interpreter

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "../catch.hpp"

#ifndef CATCH_CONFIG_MAIN
#include <cstdio>

void Interpreter(FILE* fd);
void* ReadToken(FILE* fd);

int main(int argc, char** argv)
{
    assert(argc >= 2);
    auto fd = fopen(argv[1], "rb");
    Interpreter(fd);
    fclose(fd);
}

class Token
{
public:
    char* text;
};

void Interpreter(FILE* fd)
{
    while (!feof(fd))
    {
        auto token = ReadToken(fd);
    }
}

void* ReadToken(FILE* fd)
{
    int ch = getc(fd);
    return (void*) -1;
}
#endif
