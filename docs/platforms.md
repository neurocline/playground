# Platforms

Platforms are a way of life. For us, relevant platforms include Windows,
Mac OS X, Linux, and so on.

## Platform detection

This is not crystal clear because we have host compilers and cross compilers,
and we have compilers that run on multiple platforms (GCC, Clang). Here
are the possibilities I know of

- Windows: Visual Studio, Clang (rare), GCC (rare)
- Linux: GCC, Clang
- Mac OS X: Clang, GCC (deprecated)

This means in practice, we sometimes first test for compiler, then for that compiler's
idea of platform. This is complicated when compilers are used on multiple platforms.

The main pattern for platform is this:

```
#if defined(_WIN32)
// building on some flavor of Windows)
#elif defined(defined(__APPLE__) && defined(__MACH__))
// building on Mac OS X
#elif defined(__linux__)
// building on Linux
#elif defined(__unix__)
// building on any Unix (probably BSD)
#endif
```

The main pattern for compiler is this:

```
#if defined(__clang__)
// building on Clang/LLVM
#elif defined(_MSC_VER)
// building on Visual Studio
#elif defined(__GNUC__)
// building on GCC
#endif
```

A good source, although overkill, is the Boost Predef library. Since it tries to work
on every compiler and platform known to the world, it has a ton of edge cases that are
almost certainly not relevant to you.

### Visual Studio

- `_MSC_VER` defined when building under the Visual Studio compiler.
- `_M_IX86` defined when compilation target is x86.
- `_M_X64` defined when compilation target is x64.

### GCC

- `__GNUC__`, `__GNUC_MINOR__`, `__GNUC_PATCHLEVEL__` defined by all GNU compilers
- `__i386__` defined when compilation target is x32
- `__x86_64__` defined when compilation target is x64

Combine three version macros into a single `GCC_VERSION` for easier testing:

```
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
…
/* Test for GCC > 3.2.0 */
#if GCC_VERSION > 30200
...
#endif
```

### Clang

Note that in general, Clang tries to be compatible with GCC, so many GCC defines also
exist in Clang.

- `__clang__` defined when compiling with Clang
- `__clang_major__`, `__clang_minor__`, `__clang_patchlevel__` defined when compiling with Clang, combine to get Clang version number
- `__i386__` defined when compilation target is x32
- `__x86_64__` defined when compilation target is x64

Combine three version macros into a single `CLANG_VERSION` for easier testing:

```
#define CLANG_VERSION (__clang_major__ * 10000 \
                     + __clang_minor__ * 100 \
                     + __clang_patchlevel__)
…
/* Test for Clang >= 4.1.0 */
#if CLANG_VERSION >= 40100
...
#endif
```

### Windows

- `_WIN32` Defined as 1 when the compilation target is 32-bit ARM, 64-bit ARM, x86, or x64. Otherwise, undefined.
- `_WIN64` Defined as 1 when the compilation target is 64-bit ARM or x64. Otherwise, undefined.
- `__CYGWIN__` defined when building on Windows with POSIX Cygwin.

```
#ifdef _WIN32
... code specific to Windows (32-bit or 64-bit processor)
#endif
```

Some features are not detected, they are set with macros before including `<Windows.h>`.
This eventually leads to the best practice of never including `<Windows.h>` in public
header files, since you pollute the caller with your decision on Windows features.

### Unix

- `__unix__` true for all Linux and BSD targets (but not Mac OS X)

To detect any Unix-style OS:

```
#if !defined(_WIN32) && (defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)))
    /* UNIX-style OS. ------------------------------------------- */
#endif
```

### Linux

- `__linux__` is true for Linux targets

### BSD

- `__FreeBSD__` true for FreeBSD targets
- `__NetBSD__` true for NetBSD targets
- `__OpenBSD__` true for OpenBSD targets

On BSD distributions, and only BSD distributions, including `<sys/param.h>` will
define a `BSD` macro with the OS version.

### Mac OS X

- `__APPLE__` defined when targetting Mac
- `__MACH__` defined when targetting Mac OS X

Note that Mac OS X is based in part on a fork of FreeBSD; however, `__unix__` is not
defined. Test for the combination of `__APPLE__` and `__MACH__`.

### POSIX

This can be detected by including `<unistd.h>` on a Unix-style system and checking
for a `_POSIX_VERSION` define.

```
#if !defined(_WIN32) && (defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)))
    /* UNIX-style OS. ------------------------------------------- */
#include <unistd.h>
#if defined(_POSIX_VERSION)
    /* POSIX compliant */

#endif
#endif
```

# Reference

[Predefined Macros (VS2015)](https://msdn.microsoft.com/en-us/library/b0084kay.aspx)

[Pre-defined C/C++ Compiler Macros](https://sourceforge.net/p/predef/wiki/Home/)

[Common Predefined Macros](https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html)

[Clang Language Extensions](https://clang.llvm.org/docs/LanguageExtensions.html)

[C/C++ tip: How to list compiler predefined macros](http://nadeausoftware.com/articles/2011/12/c_c_tip_how_list_compiler_predefined_macros)

[boost/predef](http://www.boost.org/doc/libs/1_62_0/boost/predef/)

[boostorg/predef](https://github.com/boostorg/predef)
