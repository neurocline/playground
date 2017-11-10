// main.cpp

#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <vector>

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

int main(int argc, char** argv)
{
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

#if _HRC
void HRC()
{
    using namespace std::chrono;
    std::cout << "high_resolution_clock timebase " << high_resolution_clock::period::num << "/" << high_resolution_clock::period::den << std::endl;

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    //std::cout << "printing out 1000 stars...\n";
    //for (int i=0; i<1000; ++i) std::cout << "*";
    //std::cout << std::endl;
    for (int i = 0; i < 1000; i++)
        high_resolution_clock::time_point t2 = high_resolution_clock::now();

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

    std::cout << "It took me " << time_span.count() * 1000000000 << " nanoseconds.";
    std::cout << std::endl;
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
