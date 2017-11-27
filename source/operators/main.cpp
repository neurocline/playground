// main.cpp

#include "operators.h"

void SimpleOps();
void destructors();

int main(int /*argc*/, char** /*argv*/)
{
    SimpleOps();
    destructors();

	return 0;
}

void SimpleOps()
{
    SimpleOp one; // SimpleOp default constructor
    one = 1; // SimpleOp copy-assignment operator from int

    SimpleOp two; // SimpleOp default constructor
    two = one; // SimpleOp copy-assignment operator

    SimpleOp three = one; // SimpleOp copy constructor

    three(); // SimpleOp function call operator

    two += one; // SimpleOp operator+=
    three -= one; // SimpleOp operator-=
    three *= one; // SimpleOp operator*=
    three /= one; // SimpleOp operator/=
    three %= two; // SimpleOp operator%=

    three = one + two; // SimpleOp operator+
    three = two - one; // SimpleOp operator-
    three = two * one; // SimpleOp operator*
    three = one / two;
    three = two % one;

    if (three) 0; // SimpleOp operator bool
    three = -two; // SimpleOp operator- (unary)
    three = +two; // SimpleOp operator+ (unary)
    if (!two) 0; // SimpleOp operator! (unary)
    three = ~two; // SimpleOp operator! (unary)
}


class A
{
public:
    virtual ~A() = 0;
};
A::~A()
{
    std::cout << "~A()\n";
}

class B: public A
{
public:
    ~B() { std::cout << "~B()\n"; }
};

void destructors()
{
    B b;

    A* a = new B();
    delete a;
}
