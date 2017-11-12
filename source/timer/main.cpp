// main.cpp

#include "Timing.h"

#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <vector>
#include <assert.h>

#include <Windows.h>

#define _QPF 1
#define _QPC 1
#define _HRC 1
#define _TSC 1
#define _TIMER 1
void QPF();
void QPC();
void HRC();
void TSC();
void TimerTest();
void test();
void TestLongDivide();

int main(int /*argc*/, char** /*argv*/)
{
    //test();
    TestLongDivide();

    #if _QPF
    QPF();
    #endif

    #if _QPC
    QPC();
    #endif

    #if _HRC
    HRC();
    #endif

    #if _TSC
    TSC();
    #endif

    #if _TIMER
    TimerTest();
    #endif

    return 0;
}

#if 0
LARGE_INTEGER nanoTicks; // this is really a 32.32 fraction of nanos per tick
void BadInitFastNanos()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    nanoTicks.QuadPart = 0;
    nanoTicks.HighPart = 1000000000L; // multiply by 2^32
    nanoTicks.QuadPart /= freq.QuadPart;
}

uint64_t BadFastNanos()
{
    LARGE_INTEGER ctr;
    QueryPerformanceCounter(&ctr);

    uint64_t d = (uint64_t) ctr.LowPart * nanoTicks.LowPart;
    uint64_t c = (uint64_t) ctr.HighPart * nanoTicks.LowPart;
    uint64_t b = (uint64_t) ctr.LowPart * nanoTicks.HighPart;
    uint64_t a = (uint64_t) ctr.HighPart * nanoTicks.HighPart;

    uint64_t v = (a << 32) + b + c + (d >> 32);
    return v;
}
#endif

void TestFast()
{
    int64_t freq = 3914060;
    int64_t billion = 1'000'000'000LL;

    LARGE_INTEGER nanoTicks;
    nanoTicks.QuadPart = 0;
    nanoTicks.HighPart = (int) billion; // multiply by 2^32
    nanoTicks.QuadPart /= freq;

//    double nanoF = (double) billion / (double) freq;
//    double two32 = (double) (1LL << 32);

//    int64_t maxcount = int64_t(0x7FFF'FFFF'FFFF'FFFFLL / billion);

    LARGE_INTEGER intF;
    intF.QuadPart = 0xFFLL;
    LARGE_INTEGER fracF;
    fracF.QuadPart = 0x7D3B3CCDFC1006E2LL;

    LARGE_INTEGER counter;
    for (counter.QuadPart = 0; counter.QuadPart < 1'000'000'000'000'000LL; counter.QuadPart += 104729*64)
    {
        // Plauger method
        int64_t whole = (counter.QuadPart / freq) * billion;
        int64_t part = (counter.QuadPart % freq) * billion / freq;
        int64_t v1 = whole + part;

        // fastmul method
 //       uint64_t d = (uint64_t) counter.LowPart * nanoTicks.LowPart;
 //       uint64_t c = (uint64_t) counter.HighPart * nanoTicks.LowPart;
 //       uint64_t b = (uint64_t) counter.LowPart * nanoTicks.HighPart;
 //       uint64_t a = (uint64_t) counter.HighPart * nanoTicks.HighPart;
 //       int64_t v2 = int64_t((a << 32) + b + c + (d >> 32));

        // direct double method
//        int64_t v3 = int64_t(counter.QuadPart * nanoF);

        // long direct double
//        int64_t v4 = int64_t(counter.HighPart * nanoF * two32 + counter.LowPart * nanoF);

        /// long fraction
        uint64_t c0w1 = (uint64_t) counter.LowPart * fracF.HighPart;
        uint64_t c0w2 = (uint64_t) counter.LowPart * intF.LowPart;
        uint64_t c1w0 = (uint64_t) counter.HighPart * fracF.LowPart;
        uint64_t c1w1 = (uint64_t) counter.HighPart * fracF.HighPart;
        uint64_t c1w2 = (uint64_t) counter.HighPart * intF.LowPart;

        int64_t v5 = (c0w1 >> 32) + c0w2 + (c1w0 >> 32) + c1w1 + (c1w2 << 32)
                     + (((c0w1 & 0xFFFFFFFF) + (c1w0 & 0xFFFFFFFF) + 0x80000000) >> 32);

//        int64_t delta12 = v2 - v1;
//        assert(delta12 > -5 && delta12 < 5);

//        int64_t delta13 = v3 - v1;
//        assert(delta13 > -2 && delta13 < 2);

//        int64_t delta14 = v4 - v1;
//        assert(delta14 > -2 && delta14 < 2);

        int64_t delta15 = v5 - v1;
        assert(delta15 > -2 && delta15 < 2);
    }
}

#if _HRC
void HRC()
{
    //TestFast();
    InitFastNanos();

    using namespace std::chrono;
    std::cout << "high_resolution_clock timebase " << high_resolution_clock::period::num << "/" << high_resolution_clock::period::den << std::endl;

    high_resolution_clock::time_point to = high_resolution_clock::now();
    uint64_t no = FastNanos();
    std::cout << "FastNanos: " << no << "   high_resolution_clock: " << to.time_since_epoch().count() << std::endl;
    std::cout << "delta: " << no - to.time_since_epoch().count() << std::endl;

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    high_resolution_clock::time_point t2;
    for (int i = 0; i < 1'000'000; i++)
        t2 = high_resolution_clock::now();
    t2 = high_resolution_clock::now();

    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    double nanoTime = time_span.count() * 1'000'000'000 / 1'000'000;
    std::cout << "high_resolution_clock: " << nanoTime << "ns (elapsed: " << int(time_span.count() * 1'000'000'000) << ")" << std::endl;
    std::cout << std::endl;

    // fast clock
    uint64_t firstnano = FastNanos();
    uint64_t nano;
    t1 = high_resolution_clock::now();
    for (int i = 0; i < 1'000'000; i++)
        nano = FastNanos();
    t2 = high_resolution_clock::now();

    time_span = duration_cast<duration<double>>(t2 - t1);
    nanoTime = time_span.count() * 1'000'000'000 / 1'000'000;
    std::cout << "FastNanos: " << nanoTime << "ns (elapsed: " << (nano - firstnano) << ")" << std::endl;
}
#endif

#if _QPF
void QPF()
{
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);
    std::cout << "QueryPerformanceFrequency = " << Frequency.QuadPart << std::endl;
}
#endif

#if _QPC
void QPC()
{
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);

    std::vector<int> skips;
    std::vector<long long> deltas;
    skips.reserve(1000);
    deltas.reserve(1000);

    LARGE_INTEGER Start;
    int startCount = 0;
    QueryPerformanceCounter(&Start);
    for (int i = 0; i < 1*1000; i++)
    {
        LARGE_INTEGER End;
        QueryPerformanceCounter(&End);
        LARGE_INTEGER Delta;
        Delta.QuadPart = End.QuadPart - Start.QuadPart;
        if (Delta.QuadPart != 0)
        {
            // std::cout << i - startCount << " - " << Delta.QuadPart << std::endl;
            deltas.push_back(Delta.QuadPart);
            skips.push_back(i);
            startCount = i;
            Start.QuadPart = End.QuadPart;
        }
    }

    for (int i = 0; ; i++)
    {
        if (i == skips.size())
            break;
        std::cout << skips[i] << " - " << deltas[i] << std::endl;
    }

    QueryPerformanceCounter(&Start);
    LARGE_INTEGER End;
    for (int i = 0; i < 100*1000; i++)
        QueryPerformanceCounter(&End);
    LARGE_INTEGER Delta;
    Delta.QuadPart = End.QuadPart - Start.QuadPart;
    std::cout << Delta.QuadPart << std::endl;
}
#endif

#if _TSC
void TSC()
{
    std::vector<long long> tscs;
    std::vector<long long> qpcs;
    tscs.reserve(1000);
    qpcs.reserve(1000);

    LARGE_INTEGER Start;
    QueryPerformanceCounter(&Start);
    long long tscStart = __rdtsc();

    for (int i = 0; i < 1*1000; i++)
    {
        LARGE_INTEGER End;
        QueryPerformanceCounter(&End);
        LARGE_INTEGER Delta;
        Delta.QuadPart = End.QuadPart - Start.QuadPart;

        if (Delta.QuadPart != 0)
        {
            long long tscv = __rdtsc();
            tscs.push_back(tscv - tscStart);
            qpcs.push_back(Delta.QuadPart);
            Start.QuadPart = End.QuadPart;
            tscStart = tscv;
        }
    }

    for (int i = 0; ; i++)
    {
        if (i == tscs.size())
            break;
        std::cout << qpcs[i] << " - " << tscs[i] << std::endl;
    }

    tscStart = __rdtsc();
    long long tscEnd;
    for (int i = 0; i < 1000*1000; i++)
    {
        tscEnd = __rdtsc();
    }
    long long tscDelta;
    tscDelta = tscEnd - tscStart;
    std::cout << tscDelta << std::endl;
}
#endif

#if 0
class ChronoTimer
{
public:
    using clock = std::chrono::high_resolution_clock;
    using time_point = std::chrono::time_point<clock>;

    ChronoTimer() : start(clock::now()) {}

    // Returns elapsed time in seconds for this clock. Takes about 50 nanoseconds on a
    // Core i7-6700K at 4 Ghz running Windows 10.
    double elapsed() const
    {
        auto delta = clock::now() - start;
        std::chrono::duration<double> span = std::chrono::duration_cast<std::chrono::duration<double>>(delta);
        return span.count();
    }

    // Return the current time as a raw time_point. Takes about 30 nanoseconds on a
    // Core i7-6700K at 4 Ghz running Windows 10
    time_point now() { return clock::now(); }

    // Get the raw time_point value for this clock
    operator time_point() { return start; }

    // Restart the timer
    void reset() { start = clock::now(); }

    // Set the timer to a specific time value (or with the time_point cast, from another ChronoTimer)
    void set(time_point v) { start = v; }

private:
    time_point start;
};
#endif

void TimerTest()
{
    ChronoTimer t;
    ChronoTimer::time_point v;
    double d;
    for (int i = 0; i < 100*1000; i++)
        //v = ChronoTimer::clock::now();
        d = t.elapsed();

    std::cout << "This took " << t.elapsed() * 1000000 << " microseconds" << std::endl;
}
