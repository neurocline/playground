// main.cpp

#include "operators.h"

int main(int /*argc*/, char** /*argv*/)
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

    if (three) ; // SimpleOp operator bool
    three = -two; // SimpleOp operator- (unary)
    three = +two; // SimpleOp operator+ (unary)
    if (!two) ; // SimpleOp operator! (unary)
    three = ~two; // SimpleOp operator! (unary)

	return 0;
}
