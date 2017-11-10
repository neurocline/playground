# File I/O

All the myriad ways to to file I/O - C, C++, Windows, Mac, Linux

## C File I/O

```
std::FILE* fopen( const char* filename, const char* mode );
int fclose( std::FILE* stream );
std::size_t fread( void* buffer, std::size_t size, std::size_t count, std::FILE* stream );
std::size_t fwrite( const void* buffer, std::size_t size, std::size_t count, std::FILE* stream );
```

If `size_t` is a 32-bit value, then files are limited to 4 GB (or 2 GB 0 on old versions
of C libraries). If `size_t` is a 64-bit value, then file size can be up to 18000 PB,
although practical limitations kick in (not all file systems support such large files).

This is a buffered I/O subsystem.

Windows stdio supports extra flags; see [fopen, wfopen](https://msdn.microsoft.com/en-us/library/yeby3zcb.aspx).

However... all this is moot, because `fseek` and `ftell` use signed int, so getting and positioning
in files is limited to the first 2 GB. So using C File I/O is rather pointless in the modern age.

## C++ File I/O

```
std::ifstream
std::ofstream
```

Must set buffering to get equivalent speed. Evidently on some platforms (at least MSVC), if you
don't set a buffer it does byte-at-a-time copies, which is insane. This is needed for both ofstream
and ifstream.

```
    void* rdbuf = std::malloc(65536);
    f.rdbuf()->pubsetbuf((char*)rdbuf, 65536);
```
