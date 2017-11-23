// rdtsc.cpp

// Timing is of course to be determined externally (usually by comparing a known clock
// to elapsed rdtsc time).

// Maybe we just want to use __rdtsc and only provide an inline function or
// macro for Clang?

// For Clang, we can use __builtin_readcyclecounter to access the x86 RDTSC instruction.
// clang also has __rdtsc() available from x86intrin.h header.
#if defined(__clang__) && defined(__has_builtin) && __has_builtin(__builtin_readcyclecounter)
inline uint64_t rdtsc()
{
	return __builtin_readcyclecounter();
}
// alternatively:
// #define rdtsc() __builtin_readcyclecounter()

// For GCC, we can use an intrinsic or inline assembly
// TBD hoist definition out so that we don't need a header include
#elif defined(__GCC__)
#include <x86intrin.h>
inline uint64_t rdtsc()
{
	return __rdtsc();
}
// alternatively:
// #define rdtsc() __rdtsc()

// On Visual Studio and Windows, we can use __rdtsc() to access the RDTSC instruction
#elif defined(_WIN32) && defined(_MSC_VER)
#pragma intrinsic(__rdtsc)  
inline uint64_t rdtsc()
{
	return __rdtsc();
}
// alternatively:
// #define rdtsc() __rdtsc()

// Otherwise, it doesn't exist. This needs to be an error if anyone actually
// uses the instruction on such a build. Figure out how to do this.
// TBD once we have ARM targets, we need to use something other than rdtsc()
#else
#error "No rdtsc instruction access available"
#endif

// GCC style inline assembly
#if 0
inline uint64_t rdtsc()
{
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}
#endif
