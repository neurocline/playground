// operators.h
//
// Demonstration of operator overloading for every operator.
// http://en.cppreference.com/w/cpp/language/operators

#pragma once

#include <iostream>

// ======================================================================================
// SimpleOp
// ======================================================================================

class SimpleOp
{
public:

    // -----------------------------------------------------------------------

    // Default constructor
    SimpleOp();

    // Destructor
    ~SimpleOp() noexcept;

    // Copy constructor
    SimpleOp(const SimpleOp& rhs);

    // Move constructor
    SimpleOp(SimpleOp&& rhs) noexcept;

    // -----------------------------------------------------------------------
    using T = SimpleOp;

    // Copy assignment operators
    T& operator=(const T& rhs);
    T& operator=(const int& rhs);

    // Move assignment operator
    T& operator=(T&& rhs) noexcept;

    // stream insertion (friend function)
    friend std::ostream& operator<<(std::ostream& os, const T& rhs);

    // stream extraction (friend function)
    friend std::istream& operator>>(std::istream& is, T& rhs);

    // function call operator that takes no parameters and returns void
    void operator()();

    // prefix increment ++val
    T& operator++();

    // postfix increment val++
    T operator++(int);

    // logical operators
    explicit operator bool() const noexcept;
    bool operator!(); // boolean negation, opposite of operator bool
    bool operator&&(const T& rhs);
    bool operator||(const T& rhs);

    // unary operators ~ - +
    T& operator~();
    T& operator-();
    T& operator+();

    // comparison
    bool operator==(const T& rhs);
    bool operator!=(const T& rhs);

    bool operator< (const T& rhs);
    bool operator> (const T& rhs);
    bool operator<=(const T& rhs);
    bool operator>=(const T& rhs);

    // compound arithmetic assignment operators += -= *= /= %=
    T& operator+=(const T& rhs);
    T& operator-=(const T& rhs);
    T& operator*=(const T& rhs);
    T& operator/=(const T& rhs);
    T& operator%=(const T& rhs);

    // compound bitwise assignment operators &= |= ^= <<= >>=
    T& operator&=(const T& rhs);
    T& operator|=(const T& rhs);
    T& operator^=(const T& rhs);
    T& operator<<=(int rhs);
    T& operator>>=(int rhs);

    // arithmetic operators + - * / %
    #if 0
    // class member functions - two copies per operation
    T& operator+(const T& rhs)
    {
        std::cout << "SimpleOp operator+\n";
        return T(*this) += rhs;
    }
    T& operator-(const T& rhs)
    {
        std::cout << "SimpleOp operator-\n";
        return T(*this) -= rhs;
    }
    T& operator*(const T& rhs)
    {
        std::cout << "SimpleOp operator*\n";
        return T(*this) *= rhs;
    }
    T& operator/(const T& rhs)
    {
        std::cout << "SimpleOp operator/\n";
        return T(*this) /= rhs;
    }
    T& operator%(const T& rhs)
    {
        std::cout << "SimpleOp operator%\n";
        return T(*this) %= rhs;
    }

    #else
    // friend functions - one less copy per invocation (one copy + two moves)
    // (friends inline in class definition are hidden from ADL resolution;
    // passing by value gets rid of the need for a temporary and helps chained operation)
    friend T operator+(T lhs, const T& rhs)
    {
        std::cout << "SimpleOp operator+ friend\n";
        lhs += rhs;
        return lhs; // return the result by value (uses move constructor)
    }
    friend T operator-(T lhs, const T& rhs)
    {
        std::cout << "SimpleOp operator- friend\n";
        lhs -= rhs;
        return lhs; // return the result by value (uses move constructor)
    }
    friend T operator*(T lhs, const T& rhs)
    {
        std::cout << "SimpleOp operator* friend\n";
        lhs *= rhs;
        return lhs; // return the result by value (uses move constructor)
    }
    friend T operator/(T lhs, const T& rhs)
    {
        std::cout << "SimpleOp operator/ friend\n";
        lhs /= rhs;
        return lhs; // return the result by value (uses move constructor)
    }
    friend T operator%(T lhs, const T& rhs)
    {
        std::cout << "SimpleOp operator% friend\n";
        lhs %= rhs;
        return lhs; // return the result by value (uses move constructor)
    }
    #endif

    // -----------------------------------------------------------------------

private:
    int value;
};

// stream insertion/extraction - can't be a member function
// (typically this would be in a separate header file so that you don't pull
// in <iostream> unless you need it.
std::ostream& operator<<(std::ostream& os, const SimpleOp& rhs);
std::istream& operator>>(std::istream& is, SimpleOp& rhs);

// ======================================================================================
// ComplexOp
// ======================================================================================

class ComplexOp
{
public:
    using T = ComplexOp;
    using value_t = char;

    // -----------------------------------------------------------------------

    // Default constructor
    ComplexOp();

    // -----------------------------------------------------------------------

    // Copy assignment operator
    T& operator=(const T& other);

    // Move assignment operator
    T& operator=(T&& other) noexcept;

    // Array subscript (return by value for simple types, or by reference for complex types)
          value_t& operator[](std::size_t idx);
    const value_t& operator[](std::size_t idx) const;

    // -----------------------------------------------------------------------

    int len;
    char* data;
};

// Notes
//
// Rule of Three: if a class defines one or more of a destructor, copy constructor,
// or copy assignment operator, it should define all three. This is a pre-C++11 rule.
//
// Law of The Big Two: if RAII is used for class data management, the custom destructor
// may be omitted.
//
// Rule of Five adds move constructor and move assignment operator to the list.
//
// Copy constructors and copy assignment operators are only needed for objects
// that own some or all of their storage. In general, this means that classes
// that only contain POD data or objects with their own storage management do
// not need copy constructors or copy assignment operators.
//
// This also applies for move constructors and move assignment operators.
//
// Move constructor and move assignment operator won't be compiler-generated for a class that
// explicitly declares any of the other special member functions.

// Copy constructor and copy assignment operator won't be compiler-generated for a class that
// explicitly declares a move constructor or move assignment operator
//
// A class with a explicitly declared destructor and implicitly defined copy constructor
// or implicitly defined copy assignment operator is considered deprecated; use =default
// to get the compiler-default code.

// In C++17 it's all or nothing; defining any of the "big five" means that none of the
// rest will be compiler-generated.

// A class with a deleted copy constructor and copy assignment operator is useful
// in that objects can be created but not copied.

