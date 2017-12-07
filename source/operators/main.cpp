// main.cpp

#include "operators.h"

void SimpleOps();
void destructors();
void bitfields();

int main(int /*argc*/, char** /*argv*/)
{
    SimpleOps();
    destructors();
    bitfields();

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

void bitfields()
{
    struct A
    {
        uint32_t local : 1;
        int32_t sign : 1;
        uint32_t len : 30;
    };

    A a;
    a.local = 1;
    a.sign = 0;
    a.len = 5;

    A b;
    b.local = 0;
    b.sign = -1;
    b.len = 10;

    int32_t sign = -1;
    b.sign = sign;

    sign = b.sign;

    struct B
    {
        uint32_t local : 1;
        int32_t sign : 1;

        union
        {
            struct
            {
                uint32_t len : 4;
                uint32_t pad : 26;
                uint32_t buf[7];
            } small;
            struct
            {
                int32_t len : 30;
                int32_t bufsize;
                uint32_t* buf;
            } big;
        };
    };

    int x = sizeof(B);
}
