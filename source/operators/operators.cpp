// operators.cpp

#include "operators.h"

#include <algorithm>
#include <iostream>

// ======================================================================================
// SimpleOp
// ======================================================================================

// Default constructor
SimpleOp::SimpleOp()
{
    std::cout << "SimpleOp default constructor\n";
    value = 0;
}

// Destructor
SimpleOp::~SimpleOp() noexcept
{
    std::cout << "SimpleOp destructor\n";
}

// Copy constructor
SimpleOp::SimpleOp(const SimpleOp& rhs)
{
    std::cout << "SimpleOp copy constructor\n";
    value = rhs.value;
}

// Move constructor
SimpleOp::SimpleOp(T&& rhs) noexcept
{
    std::cout << "SimpleOp move constructor\n";
    value = rhs.value;
    rhs.value = 0;
}

// -----------------------------------------------------------------------

// copy-assignment operator
// - no action on self-assignment
// - return lhs by reference
SimpleOp& SimpleOp::operator=(const SimpleOp& rhs)
{
    std::cout << "SimpleOp copy-assignment operator\n";

    // no action on self-assignment
    if (this == &rhs)
        return *this;

    // A SimpleOp doesn't have allocated storage, so we have nothing to "free"
    /* free(this.value) */
    /* this.value = SimpleOp{} */

    // Make a copy of rhs.value
    this->value = rhs.value;

    // Always return ourself so we can chain operations
    return *this;
}

SimpleOp& SimpleOp::operator=(const int& rhs)
{
    std::cout << "SimpleOp copy-assignment operator from int\n";

    value = rhs;
    return *this;
}

// move-assignment operator
SimpleOp& SimpleOp::operator=(SimpleOp&& rhs) noexcept
{
    std::cout << "SimpleOp move-assignment operator\n";
    value = rhs.value;

    return *this;
}

// stream insertion - can't be a member function
// (typically this would be in a separate header file so that you don't pull
// in <iostream> unless you need it.
std::ostream& operator<<(std::ostream& os, const SimpleOp& rhs)
{
    std::cout << "SimpleOp write to stream\n";

    // write obj to stream
    os << rhs.value;

    // Always return ourself so we can chain operations
    return os;
}

// stream extraction - can't be a member function
// (typically this would be in a separate header file so that you don't pull
// in <iostream> unless you need it.
std::istream& operator>>(std::istream& is, SimpleOp& rhs)
{
    std::cout << "SimpleOp read from stream\n";

    // read obj from stream
    is >> rhs.value;

    // if (T could not be constructed)
    //    is.setstate(std::ios::failbit);

    // Always return ourself so we can chain operations
    return is;
}

// operator function call
void SimpleOp::operator()()
{
    std::cout << "SimpleOp operator function call with no parameters and void return\n";
}

// operator++ prefix increment (++val)
SimpleOp& SimpleOp::operator++()
{
    std::cout << "SimpleOp operator++ prefix (++val)\n";
    value += 1;
    return *this;
}

// operator++ postfix increment (val++)
SimpleOp SimpleOp::operator++(int)
{
    std::cout << "SimpleOp operator++ postfix (val++)\n";

    // Save the value before increment
    SimpleOp pre(*this);
    operator++();

    // Return value before increment
    return pre;
}

// -----------------------------------------------------------------------
// Logical operators

SimpleOp::operator bool() const noexcept
{
    std::cout << "SimpleOp operator bool\n";
    return value ? true : false;
}

bool SimpleOp::operator!()
{
    std::cout << "SimpleOp operator!\n";
    return !operator bool();
}

// loses short-circuit behavior
bool SimpleOp::operator&&(const SimpleOp& rhs)
{
    return value && rhs.value;
}

// loses short-circuit behavior
bool SimpleOp::operator||(const SimpleOp& rhs)
{
    return value || rhs.value;
}

// -----------------------------------------------------------------------
// Unary operators

SimpleOp& SimpleOp::operator~()
{
    std::cout << "SimpleOp operator~\n";
    SimpleOp{*this};
    value = ~value;
    return *this;
}

SimpleOp& SimpleOp::operator+()
{
    std::cout << "SimpleOp operator+ (unary)\n";
    SimpleOp{*this};
    value = +value;
    return *this;
}

SimpleOp& SimpleOp::operator-()
{
    std::cout << "SimpleOp operator- (unary)\n";
    SimpleOp{*this};
    value = -value;
    return *this;
}

// -----------------------------------------------------------------------
// Comparison operators

bool SimpleOp::operator==(const SimpleOp& rhs)
{
    std::cout << "SimpleOp operator==\n";
    return value == rhs.value;
}

bool SimpleOp::operator!=(const SimpleOp& rhs)
{
    std::cout << "SimpleOp operator!=\n";
    return !(*this == rhs);
}

bool SimpleOp::operator<(const SimpleOp& rhs)
{
    std::cout << "SimpleOp operator<\n";
    return value < rhs.value;
}

bool SimpleOp::operator>(const SimpleOp& rhs)
{
    std::cout << "SimpleOp operator>\n";
    return *(const_cast<SimpleOp*>(&rhs)) < *this;
}

bool SimpleOp::operator<=(const SimpleOp& rhs)
{
    std::cout << "SimpleOp operator<=\n";
    return !(*this > rhs);
}

bool SimpleOp::operator>=(const SimpleOp& rhs)
{
    std::cout << "SimpleOp operator>=\n";
    return !(*this < rhs);
}

// -----------------------------------------------------------------------
// Arithmetic binary operators

// operator+=
SimpleOp& SimpleOp::operator+=(const SimpleOp& rhs)
{
    std::cout << "SimpleOp operator+=\n";
    value += rhs.value;
    return *this;
}

// operator-=
SimpleOp& SimpleOp::operator-=(const SimpleOp& rhs)
{
    std::cout << "SimpleOp operator-=\n";
    value -= rhs.value;
    return *this;
}

// operator*=
SimpleOp& SimpleOp::operator*=(const SimpleOp& rhs)
{
    std::cout << "SimpleOp operator*=\n";
    value *= rhs.value;
    return *this;
}

// operator/=
SimpleOp& SimpleOp::operator/=(const SimpleOp& rhs)
{
    std::cout << "SimpleOp operator/=\n";
    value /= rhs.value;
    return *this;
}

// operator%=
SimpleOp& SimpleOp::operator%=(const SimpleOp& rhs)
{
    std::cout << "SimpleOp operator%=\n";
    value %= rhs.value;
    return *this;
}

// ======================================================================================
// ComplexOp
// ======================================================================================

// copy-assignment operator
// - no action on self-assignment
// - return lhs by reference
ComplexOp& ComplexOp::operator=(const ComplexOp& other)
{
    std::cout << "ComplexOp copy-assignment operator\n";

    // no action on self-assignment
    if (this == &other)
        return *this;

    // Allocate new storage if necessary
    if (len != other.len)
    {
        delete [] data;
        data = nullptr;
        data = other.len ? new char[other.len] : nullptr;
        len = other.len;
    }

    // Copy from rhs
    std::copy(other.data, other.data + len, data);

    // Always return ourself so we can chain operations
    return *this;
}
