// Timing.h

#if __has_include(<chrono>)

#include <chrono>

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

//int64_t perf_counter();
//int64_t time();

// Supports int64_t (nanoseconds) and double (seconds)
template<typename T>
auto perf_counter() -> T;
