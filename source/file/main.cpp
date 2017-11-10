// main.cpp

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <fstream>

#include "timer.h"

#define CFILE 0
#define CPPFILE 1

void CFILE_Test();
void CPPFILE_Test();

int main(int argc, char**argv)
{
    #if CFILE
    CFILE_Test();
    #endif

    #if CPPFILE
    CPPFILE_Test();
    #endif
}

#if CFILE
void CFILE_Test_Write()
{
    std::cout << "CFILE_Test_Write" << std::endl;
    ChronoTimer t;

    // Create a very large file. On a Core i7-6700K with an SSD this
    // writes at 443 MB/sec.
    FILE* f = std::fopen("largefile.bin", "wb");
    assert(f != NULL);
    void* buf = std::malloc(1024*1024);
    std::memset(buf, 0, 1024*1024);

    for (long* b = (long*)buf; b < (long*)buf + 256*1024 - 4; b++)
        b[1] = b[0] + 104729;

    for (int i = 0; i < 16 * 1024; i++)
    {
        auto written = std::fwrite(buf, 1, 1024*1024, f);
        assert(written == 1024*1024);
        if ((i % 32) == 31)
            std::cout << ".";
    }

    std::fflush(f);
    std::fclose(f);
    std::free(buf);

    std::cout << std::endl;
    std::cout << "Created 16 GB file in " << t.elapsed() << " seconds" << std::endl;
}

void CFILE_Test_Read()
{
    std::cout << "CFILE_Test_Read" << std::endl;
    ChronoTimer t;

    // Read a very large file. This reads improbably quickly, at 6.9 GB/sec.
    // Somehow, the entire 16 GB file is still in the cache? That's crazy; the machine
    // this is running on only has 32 GB of RAM. It could be that Windows noticed it was
    // all zeros and compressed it - RLE encoding can be done very quickly. So TBD change
    // the data so it's not compressible.
    // OK, made the data compressible and it still read at 6.9 GB/sec. Larger file? The
    // drive is a Samsung 850 EVO connected via SATA, so it can't actually transfer data
    // that quickly.
    FILE* f = std::fopen("largefile.bin", "rb");
    assert(f != NULL);
    void* buf = std::malloc(1024*1024);

    for (int i = 0; i < 16 * 1024; i++)
    {
        auto read = std::fread(buf, 1, 1024*1024, f);
        assert(read == 1024*1024);
        if ((i % 32) == 31)
            std::cout << ".";
    }

    std::fclose(f);
    std::free(buf);

    std::cout << std::endl;
    std::cout << "Read 16 GB file in " << t.elapsed() << " seconds" << std::endl;
}

void CFILE_Test()
{
    CFILE_Test_Write();
    CFILE_Test_Read();
}
#endif

#if CPPFILE
void CPPFILE_Test_Write()
{
    std::cout << "CPPFILE_Test_Write" << std::endl;
    ChronoTimer t;

    // Create a very large file.
    // Disappointing - this is quite a bit slower than naive C fwrite;
    // on the same machine, this writes at 304 MB/sec.
    const size_t bufsize = 1024*1024;
    const size_t filesize = 16LL*1024*1024*1024;

    void* buf = std::malloc(bufsize);
    std::memset(buf, 0, 1024*1024);

    for (long* b = (long*)buf; b < (long*)buf + 256*1024 - 4; b++)
        b[1] = b[0] + 104729;

    std::ofstream f("largefile.bin", std::ios::binary);
    assert(f.is_open());

    // Evidently on MSVC, it writes one byte at a time unless you set a buffer.
    // With this buffer, we now write at 466 MB/sec, essentially same speed as naive fwrite.
    void* rdbuf = std::malloc(65536);
    f.rdbuf()->pubsetbuf((char*)rdbuf, 65536);

    for (int i = 0; i < filesize / bufsize; i++)
    {
        f.write((char*)buf, bufsize);
        if ((i % 32) == 31)
            std::cout << ".";
    }

    f.flush();
    f.close();
    std::free(buf);
    std::free(rdbuf);

    std::cout << std::endl;
    size_t filegb = filesize / (1024*1024*1024);
    std::cout << "Created " << filegb << " GB file in " << t.elapsed() << " seconds" << std::endl;
}

void CPPFILE_Test_Read()
{
    std::cout << "CPPFILE_Test_Read" << std::endl;
    ChronoTimer t;

    // Read a large file
    const size_t bufsize = 1024*1024;
    const size_t filesize = 16LL*1024*1024*1024;

    void* buf = std::malloc(bufsize);

    // Read a large file. This reads at 1.9 GB/sec, a lot slower than naive C fread.
    std::ifstream f("largefile.bin", std::ios::binary);
    assert(f.is_open());

    // Evidently on MSVC, it reads one byte at a time unless you set a buffer. With
    // this buffer, we read at 5 GB/sec. Still not as fast as naive fread. Maybe a
    // bigger buffer would help, since we're evidenly copying from RAM. Although why
    // would it matter?
    void* rdbuf = std::malloc(65536);
    f.rdbuf()->pubsetbuf((char*)rdbuf, 65536);

    for (int i = 0; i < filesize / bufsize; i++)
    {
        f.read((char*)buf, bufsize);
        auto read = f.gcount();
        assert(read == bufsize);
        if ((i % 32) == 31)
            std::cout << ".";
    }

    f.close();
    std::free(buf);
    std::free(rdbuf);

    std::cout << std::endl;
    size_t filegb = filesize / (1024*1024*1024);
    std::cout << "Read " << filegb << " GB file in " << t.elapsed() << " seconds" << std::endl;
}

void CPPFILE_Test()
{
    CPPFILE_Test_Write();
    CPPFILE_Test_Read();
}
#endif
