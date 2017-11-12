// Timing.cpp

#include "Timing.h"

// this works, but is terrible

template<typename T>
auto perf_counter() -> T
{
    T value = 0;
    return value;
}

template<>
auto perf_counter() -> double
{
    return 1.0;
}

template<>
auto perf_counter() -> int64_t
{
    return 2;
}

#include <iostream>

void p(double v)
{
    uint64_t i = *(uint64_t*)&v;
    std::cout << v << "=" << i << "   ";
}

void pd(double v)
{
    uint64_t i = *(uint64_t*)&v;
    int64_t s = (i >> 63) & 1;
    int64_t m = (i >> 52) & 0x7FF;
    int64_t f = i & 0xFFFFFFFFFFFFFL;
    std::cout << v << "=" << s << "," << m << "," << f << "; ";
}

void test_double_range()
{
    double billion = 1'000'000'000.0;
    for (double num = 1.0; num < 10*billion; num *= 2)
    {
        double next = num;
        uint64_t* pnext = (uint64_t*)&next;
        while (next - num == 0.0)
            *pnext += 1;
        double y = num / (86400 * 365.25);
        std::cout << y << " ";
        std::cout << (num / 86400) << ": ";
        pd(num);
        pd(next);
        std::cout << next - num;
        std::cout << std::endl;
    }
}

void test()
{
    test_double_range();

    auto start_double = perf_counter<double>();
    auto end_int64 = perf_counter<int64_t>();

    double day = 86400.0;
    double year = day * 365.25;
    double billion = 1'000'000'000.0;

    for (int y = 0; y < 100; y++)
    {
        std::cout << y << ": ";
        double t1 = y*year + 0.001;
        uint64_t t1m = *(uint64_t*)&t1 & 0x000F'FFFF'FFFF'FFFFL;
        pd(t1);
        double t2 = t1 + 1'000'000.0/billion;
        uint64_t t2m = *(uint64_t*)&t2 & 0x000F'FFFF'FFFF'FFFFL;
        pd(t2);
        double d = t2 - t1;
        pd(d);
        // std::cout << (t2m - t1m);
        double nanos = d * billion;
        std::cout << (int64_t) nanos;
        std::cout << std::endl;
    }
}
