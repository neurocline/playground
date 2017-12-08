// ======================================================================================
// Num.h
// - arbitrary-precision numbers with basic operator support
// ======================================================================================

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

// Get definition of std::byte
#if defined(_MSVC_LANG) && _MSVC_LANG > 201402
// - in Visual Studio as of VS 2017 15.3 and /std:c++17
#include <cstddef>
// - in clang/libc++ as of 5.0
// Otherwise, create it
#else
namespace std {
    enum class byte : unsigned char {};
}
#endif

// ======================================================================================

class Num
{
public:
	Num();

    // Big 5: destructor, copy constructor, copy assignment operator
    // (move constructor and move assignment operator are default generated)
	~Num() noexcept;
    Num(const Num& other) noexcept;            // Copy constructor
    Num& operator=(const Num& other) noexcept; // Copy assignment operator
    Num(Num&& rhs) = default;                  // Move constructor
    Num& operator=(Num&& rhs) = default;       // Move assignment operator

    // Reserve space for a large Num. This can never ben used to shrink the size
    // of a Num - to "garbage collect", copy to a new zero-length Num.
    void reserve(int size);

    // Construct Num from integral primitives
    // Do we really need int and unsigned int?
    Num(int v) noexcept;
    Num(unsigned int v) noexcept;
    Num(long long v) noexcept;
    Num(unsigned long long v) noexcept;

    // Copy assignment Num from integral primitives
    // Do we really need int and unsigned int?
    Num& operator=(int v) noexcept;
    Num& operator=(unsigned int v) noexcept;
    Num& operator=(long long v) noexcept;
    Num& operator=(unsigned long long v) noexcept;

    // Construct a Num from a c-string (note that this can conflict with the long long
    // constructor, due to implicit promotion rules - get rid of it in favor of std::string
    // and string_view constructors?)
    Num(char const* p, int base=10);

    // Construct from a string_view
    Num(const std::string_view& s, int base=10);

    // Construct from a string
    Num(const std::string& s, int base=10);

    // Conversion operators (will return mod 2^32 or 2^64)
    #if 0
    explicit operator int() const;
    explicit operator unsigned int() const;
    explicit operator long long() const;
    explicit operator unsigned long long() const;
    #endif

    // Each operator has several variants
    // - Num op Num
    // - Num op uint32_t
    // - uint32_t op Num
    // TBD integral variants with friend operator (instead of conversion to Num which takes time)
    #define MAKE_OP(OP) \
        Num operator OP (const Num& rhs); \
        Num& operator OP##= (const Num& rhs); \
        /*Num operator OP (const uint32_t& rhs);*/ \
        /*Num& operator OP##= (const uint32_t& rhs)*/ \
        ;

    MAKE_OP(+)
    MAKE_OP(-)
    MAKE_OP(*)
    MAKE_OP(/)
    MAKE_OP(%)
    MAKE_OP(^)

    // divmod instruction that returns both remainder and quotient
    void divmod(const Num& rhs, Num& quotient, Num& remainder);
    //uint32_t divmod(uint32_t rhs, Num& quotient);

    #if 0
    // Math by a single "digit"
    Num operator%(const uint32_t& rhs);
    Num operator^(const uint32_t& rhs); // exponentiation

    Num& operator%=(const uint32_t& rhs);
    Num& operator^=(const uint32_t& rhs); // exponentiation

    // Arbitrary math - the Num result grows as needed to hold the
    // full result.
    Num operator%(const Num& rhs);
    Num operator^(const Num& rhs); // exponentiation

    Num& operator%=(const Num& rhs);
    Num& operator^=(const Num& rhs); // exponentiation

    Num& operator>>=(const int rhs);

    // Chunk-size read and write to the underlying storage, for
    // setting larger-sized values without parsing a string
    // (will grow string, be careful)
    uint32_t operator [](int i) const; // read
    uint32_t& operator [](int i); // write
    #endif

    // Compare lhs <=> rhs:
    //  -1 if lhs  < rhs
    //   0 if lhs == rhs
    //   1 if lhs  > rhs
    int magcmp(const Num& rhs) const;
    int magcmp(const uint32_t& digit) const;

    // Convert Num to integer
    uint64_t to_uint64() const; // modulo 2^64
    int64_t to_int64() const;  // modulo 2^63

    // Convert string to Num
    bool from_cstring(char const* p, int base=10);
    const Num& from_string(const std::string_view& s, int base=10);
    const Num& from_string(const std::string& s, int base=10);

    // Convert Num to string
    int to_cstring(char* p, int len, int base=10);
    std::string to_string(int base=10);

    #if 0
    // Sign of number
    enum class Sign
    {
        Positive = 0,
        Negative = 1
    };

    Sign sign() const { return data.sign ? Sign::Negative : Sign::Positive; }
    #endif

    void from_int64(long long v);
    void from_uint64(unsigned long long uv);

//private:

    // Grow a Num by the indicated number of digits. This is an internal function
    // used by math operations. This must eventually be followed by trim(), because
    // a Num must be left in normalized form, where the most-significant digit is
    // non-zero.
    uint32_t* grow(int amt = 1);

    // Shrink a Num by the indicated number of digits. This is an internal function
    // used by math operations. This will perform a modulo to digit^len, where len
    // is the length of Num at the end of the shrink operation.
    uint32_t* shrink(int amt = 1);

    // Resize a Num. This is an internal function.
    uint32_t* resize(int size);

    // Call trim to make sure the Num is in canonical form - the most significant digit
    // is non-zero, or the Num is zero length.
    void trim();

    // Return the capacity of the Num. This is an internal function used by operators
    // and buffer management.
    int capacity() { return data.local ? smallbufsize : big.bufsize; }

    // Size of current number in digits
    //int len() const { return data.len; }

    // Return a pointer to the start of Num storage. A Num is stored as an array of
    // digits. The non-template version of Num is hardcoded to `uint32_t` as the digit
    // size.
    uint32_t* databuffer() { return data.local ? small.buf : big.buf; }
    const uint32_t* cdatabuffer() const { return data.local ? small.buf : big.buf; }

    // Add lhs + rhs ignoring sign
    Num& addto(const Num& rhs);

    // Subtract lhs - rhs ignoring sign and assuming lhs >= rhs
    Num& subfrom(const Num& rhs);

    // Clear out part of a Num
    // TBD get rid of the need for this
    //void clear_(int s, int e) { memset(databuffer()+s, 0, (e - s)*sizeof(uint32_t)); }
    //void dup_(uint32_t* dest) { memcpy(dest, databuffer(), data.len*sizeof(uint32_t)); }

    void clear_digits(int b, int e) { memset(databuffer()+b, 0, (e - b)*sizeof(uint32_t)); }
    void copy_digits(uint32_t* dest, const uint32_t* src, int digits) {
        memcpy(dest, src, digits * sizeof(uint32_t));
    }

    static constexpr int smallbufsize = 7;
    union
    {
        uint32_t prefix; // this is local + sign + len
        struct
        {
            uint32_t local : 1; // set to 1 for small data optimization
            int32_t sign : 1; // 0 for positive, -1 for negative
            int32_t len : 30; // this is too big, but there's nothing else to use it for yet
        } data;
        struct
        {
            uint32_t local : 1; // set to 1 for small data optimization
            int32_t sign : 1; // 0 for positive, -1 for negative
            int32_t len : 30; // this is too big, but there's nothing else to use it for yet
            uint32_t buf[smallbufsize];
        } small;
        struct
        {
            uint32_t local : 1; // set to 0 for big data
            int32_t sign : 1; // 0 for positive, -1 for negative
            int32_t len : 30;
            int32_t bufsize;
            uint32_t* buf;
        } big;
    };
};


inline bool operator==(const Num& lhs, const Num& rhs) noexcept { return lhs.magcmp(rhs) == 0; }
inline bool operator!=(const Num& lhs, const Num& rhs) noexcept { return lhs.magcmp(rhs) != 0; }
inline bool operator<(const Num& lhs, const Num& rhs) noexcept { return lhs.magcmp(rhs) < 0; }
inline bool operator<=(const Num& lhs, const Num& rhs) noexcept { return lhs.magcmp(rhs) <= 0; }
inline bool operator>(const Num& lhs, const Num& rhs) noexcept { return lhs.magcmp(rhs) > 0; }
inline bool operator>=(const Num& lhs, const Num& rhs) noexcept { return lhs.magcmp(rhs) >= 0; }

static_assert(sizeof(Num::small) == 32, "Num::small unexpected size");
static_assert(sizeof(Num::small) >= sizeof(Num::big), "Num::small data too small!");

// --------------------------------------------------------------------------------------
// Internal Num definition

// quotient, remainder = dividend / divisor
//
// quotient - output (must be at least dividendSize - divisorSize + 1)
// remainder - output (must be at least divisorSize)
// dividend - input (dividendSize WORDs)
// divisor - input (cdivisorSize WORDs)
// dividendSize - number of WORDs in dividend
// divisorSize - number of WORDs in divisor
// scratch - n+m+1 WORDs (can be nullptr if 16 WORDS or less)
template<typename WORD>
bool MultiwordDivide(
    WORD* quotient, WORD* remainder,
    const WORD* dividend, const WORD* divisor,
    int dividendSize, int divisorSize,
    WORD* scratch = nullptr);

//
// Math terms
// addition: augend + addend
// subtraction: minuend - subtrahend
// multiply: multiplicand × multiplier
// division: dividend ÷ divisor
