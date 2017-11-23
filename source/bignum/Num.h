// Num.h

#pragma once

#include <cstdint>

// Get definition of std::byte
// - in clang/libc++ as of 5.0
// - in Visual Studio as of VS 2017 15.3 and /std:c++17
#if 0
#include <cstddef>
// Otherwise, create it
#else
namespace std {
    enum class byte : unsigned char {};
}
#endif
// this probably doesn't work because of Windows Kit defining byte
// c:\program files (x86)\windows kits\8.0\include\shared\rpcndr.h(164)
using byte = std::byte;

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

    // Chunk-size read and write to the underlying storage, for
    // setting larger-sized values without parsing a string
    uint32_t operator [](int i) const; // read
    uint32_t& operator [](int i); // write

    // Convert Num to integer
    uint64_t to_uint64() const; // modulo 2^64
    int64_t to_int64() const;  // modulo 2^63

    // Size of current number in bytes
    int len();

private:
	byte raw[16];
	static_assert(sizeof(raw) == 16, "");
};

