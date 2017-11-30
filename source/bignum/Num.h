// ======================================================================================
// Num.h
// - arbitrary-precision numbers with basic operator support
// ======================================================================================

#pragma once

#include <cstdint>

// Get definition of std::byte
// - in clang/libc++ as of 5.0
#if defined(_MSVC_LANG) && _MSVC_LANG > 201402
// - in Visual Studio as of VS 2017 15.3 and /std:c++17
#include <cstddef>
// Otherwise, create it
#else
namespace std {
    enum class byte : unsigned char {};
}
#endif
// this probably doesn't work because of Windows Kit defining byte
// c:\program files (x86)\windows kits\8.0\include\shared\rpcndr.h(164)
// Also, using at global scope is bad behavior.
using byte = std::byte;

// ======================================================================================

class Num
{
public:
	Num();
	~Num();

    // Copy constructor
    Num(const Num& other) noexcept;

    // Construct a Num from a long int
    Num(long long v);

    // Construct a Num from a c-string (note that this can conflict with the long long
    // constructor, due to implicit promotion rules - get rid of it in favor of std::string
    // and string_view constructors?)
    Num(char const* p, int base=10);

    // Copy assignment operator
    Num& operator=(const Num& other) noexcept;

    // Math by a single "digit"
    Num operator+(const uint32_t& rhs);
    Num operator-(const uint32_t& rhs);
    Num operator*(const uint32_t& rhs);
    Num operator/(const uint32_t& rhs);
    Num operator%(const uint32_t& rhs);

    Num& operator+=(const uint32_t& rhs);
    Num& operator-=(const uint32_t& rhs);
    Num& operator*=(const uint32_t& rhs);
    Num& operator/=(const uint32_t& rhs);
    Num& operator%=(const uint32_t& rhs);

    // Arbitrary math - the Num result grows as needed to hold the
    // full result.
    Num operator+(const Num& rhs);
    Num operator-(const Num& rhs);
    Num operator*(const Num& rhs);
    Num operator/(const Num& rhs);
    Num operator%(const Num& rhs);

    Num& operator+=(const Num& rhs);
    Num& operator-=(const Num& rhs);
    Num& operator*=(const Num& rhs);
    Num& operator/=(const Num& rhs);
    Num& operator%=(const Num& rhs);

    // divmod instruction that returns both remainder and quotient
    void divmod(const Num& rhs, Num& quotient, Num& remainder);
    uint32_t divmod(uint32_t rhs, Num& quotient);

    // Chunk-size read and write to the underlying storage, for
    // setting larger-sized values without parsing a string
    uint32_t operator [](int i) const; // read
    uint32_t& operator [](int i); // write

    // Convert Num to integer
    uint64_t to_uint64() const; // modulo 2^64
    int64_t to_int64() const;  // modulo 2^63

    // Convert string to Num
    bool from_cstring(char const* p, int base=10);

    // Convert Num to string
    int to_cstring(char* p, int len, int base=10);

    // Size of current number in bytes
    int len() const;

    // Sign of number
    int sign() const;

//private:
    void grow(int16_t amt = 1);
    void shrink(int16_t amt = 1);
    void resize(int16_t size);
    void trim();

    Num& addto(const Num& rhs);
    Num& subfrom(const Num& rhs);
    int magcmp(const Num& rhs);
    int magcmp(const uint32_t& digit);

    int to_cstring_len(int base=10);

	byte raw[32];
    // static_assert(sizeof(raw) == 32, ""); // this is a compile error in VS2017 15.4.1
};

static_assert(sizeof(Num::raw) == 32, "");

// TBD can we create a make_Num helper that creates a large Num with
// contiguous storage above and beyond the default? Can we create
// different-sized local Num, maybe with a template parameter?

// --------------------------------------------------------------------------------------
// Internal Num definition

struct NumData
{
    int16_t len;
    int16_t sign;
    uint32_t data[7];
};
static_assert(sizeof(NumData) <= sizeof(Num::raw), "NumData misdefined");

template<typename WORD>
bool MultiwordDivide(
    WORD* quotient, WORD* remainder,
    const WORD* dividend, const WORD* divisor,
    int dividendSize, int divisorSize);

// TBD add dynamic data sizing
// TBD Small Object Optimization

// For small object optimization, there are several sizes to consider
// 16 bytes: 1 bit small-object, 1 bit sign, 6 bits length, 15 bytes number
// 24 bytes: 1 bit small-object, 1 bit sign, 6 bits length, 23 bytes number
// 32 bytes 1 bit small-object, 1 bit sign, 6 bits length, 31 bytes number
//
// Note that a pointer is 8 bytes on 64-bit architecture. We probably want to think
// in units of pointer-size, so that means 16-bytes is the smallest possible.
// 16 bytes = 120 bits of number, 24 bytes = 184 bits of number, 32 bytes = 248 bits of number
// 2^120 = 1.32e36 (approximately the range of 32-bit float)
// 2^184 = 2.45e55
// 2^248 = 4.52e74
// 64-bit float has range 1e-308 to 1e308 but only 53 bits of precision
// 100! = 9.3e157

//
// Math terms
// addition: augend + addend
// subtraction: minuend - subtrahend
// multiply: multiplicand × multiplier
// division: dividend ÷ divisor
