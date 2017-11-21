// main.cpp

// This only exists because the Visual C++ compiler doesn't offer a way to dump
// out all the predefined macros

#include <iostream>
#include <cassert>

#define SHOW(DEFINE) std::cout << "#define " #DEFINE " " << DEFINE << std::endl

void cstandard()
{
    #if defined(__cplusplus)
    SHOW(__cplusplus);
    #endif
    #if defined(__DATE__)
    SHOW(__DATE__);
    #endif
    #if defined(__FILE__)
    SHOW(__FILE__);
    #endif
    #if defined(__LINE__)
    SHOW(__LINE__);
    #endif
    #if defined(__STDC__)
    SHOW(__STDC__);
    #endif
    #if defined(__STDC_HOSTED__)
    SHOW(__STDC_HOSTED__);
    #endif
    #if defined(__STDCPP_THREADS__)
    SHOW(__STDCPP_THREADS__);
    #endif
    #if defined(__TIME__)
    SHOW(__TIME__);
    #endif
}

void Windows()
{
    #if defined(__COUNTER__)
    SHOW(__COUNTER__);
    #endif
    #if defined(_M_AMD64)
    SHOW(_M_AMD64);
    #endif
    #if defined(_M_IX86)
    SHOW(_M_IX86);
    #endif
    #if defined(_MSC_BUILD)
    SHOW(_MSC_BUILD);
    #endif
    #if defined(_MSC_EXTENSIONS)
    SHOW(_MSC_EXTENSIONS);
    #endif
    #if defined(_MSC_FULL_VER)
    SHOW(_MSC_FULL_VER);
    #endif
    #if defined(_MSC_VER)
    SHOW(_MSC_VER);
    #endif
    #if defined(_MSVC_LANG)
    SHOW(_MSVC_LANG);
    #endif
    #if defined(_MT)
    SHOW(_MT);
    #endif
    #if defined(_WIN32)
    SHOW(_WIN32);
    #endif
    #if defined(_WIN64)
    SHOW(_WIN64);
    #endif
}

void Clang()
{
    #if defined(__clang__)
    SHOW(__clang__);
    #endif
    #if defined(__clang_version__)
    SHOW(__clang_version__);
    #endif
    #if defined(__llvm__)
    SHOW(__llvm__);
    #endif
    #if defined(__c2__)
    SHOW(__c2__);
    #endif
    #if defined(__c2_version__)
    SHOW(__c2_version__);
    #endif
}

void GCC()
{
    #if defined(__GNUC__)
    SHOW(__GNUC__);
    #endif
    #if defined(__GNUC_MINOR__)
    SHOW(__GNUC_MINOR__);
    #endif
    #if defined(__GNUC_PATCHLEVEL__)
    SHOW(__GNUC_PATCHLEVEL__);
    #endif
    #if defined(__MINGW32__)
    SHOW(__MINGW32__);
    #endif
    #if defined(__MINGW64__)
    SHOW(__MINGW64__);
    #endif
    #if defined(__CYGWIN__)
    SHOW(__CYGWIN__);
    #endif
}

void dev()
{
    #if defined(_DEBUG)
    SHOW(_DEBUG);
    #endif
    #if defined(NDEBUG)
    SHOW(NDEBUG);
    #endif
}

int main(int /*argc*/, char** /*argv*/)
{
    cstandard();
    Windows();
    Clang();
    GCC();
    dev();
}
