// main.cpp

#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <vector>

#include <Windows.h>

#define _QPF 0
#define _QPC 0
#define _HRC 0
#define _TSC 1
void QPF();
void QPC();
void HRC();
void TSC();

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
    for (int i = 0; i < 18; i++) ;

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
