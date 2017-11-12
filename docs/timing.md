# Timing

Optimizing code for performance requires that you measure it, and carefully, because many
optimizations are counterintuitive. Measuring performance is complicated by the fact that
measurement itself can affect what you are measuring, and because the system under
measurement usually is complex and unrelated events can confuse measurement.

The overhead for all performance measurement calls varies by operating system and hardware.
In the following, we have measurements from these:

- Windows: Core i7-6700K at 4GHZ running Windows 10 Pro version 1703, Visual Studio 2017

Timing falls into two categories - the action of taking timestamps, and then a more complex
system to match timestamps with events and to process data to remove unrelated factors.

# Low-level timing

In its essence, and in actuality on most systems, this is simple. Some piece of hardware
is counting elapsed time - sample the hardware at beginning and end of a sequence and
compute elapsed time.

## C++11

C++11 added the `<chrono>` class as a cross-platform way to work with time in a generic
but performant way. Under the hood, this uses a relevant platform-specific call, almost
certainly one of the ones listed below. But if you have `<chrono>`, your code will work
on all platforms.

For performance measurements, we want to use `high_resolution_clock`.

```
using clock = std::chrono::high_resolution_clock;
using duration = std::chrono::duration<double>;
using duration_cast = std::chrono::duration_cast;
using time_point = std::chrono::time_point<clock>;

time_point start = clock::now();
... code being measured ...
double elapsed_seconds = duration_cast<duration>(clock::now() - start).count();
```

A call to `clock::now` seems to take about 30 nanoseconds on the C++17 library that comes with
Visual Studio 2017. Adding a call to `duration_cast` to convert the chrono clock to a simple
scalar of seconds turns that into about 55 nanoseconds. This is quite a bit slower than the
raw calls, but not significant for many uses.

## Windows

The only two useful timing calls on Windows at this point are `QueryPerformanceCounter`
and the `rdtsc` instruction used via the `__rdtsc` compiler intrinsic. Avoid using any
other timing calls - specifically, don't use the old multimedia timers.

### rdtsc

Intel and AMD processors have a TSC timesource, which ticks at the clock rate, or, on newer
processors, at an invariant clock rate. Before the latter, the TSC was very tricky to use,
because it changed with clock throttling or power savings modes.

The `rdtsc` instruction is straightforward, but you need to determine the clock frequency
yourself, or measure it by comparing with another known clock. In the below, we assume that
we have done this and stored the inverse of ticks/second in `sec_per_rdtsc` as a double.

```
double sec_per_rdtsc = rdtsc_timebase(); // undefined magic

long long start = __rdtsc();
... code being measured ...
long long delta_rdtsc = __rdtsc() - start;

double elapsed_seconds = delta_rdtsc * sec_per_rdtsc;
```

A call to `rdtsc` takes about 6 nanoseconds on our Windows machine.

It is possible to create a `<chrono>` clock from TSC. Howard Hinnant wrote a StackOverflow
answer sketching this out. Adapting for Visual Studio:

```
namespace x
{

struct clock
{
    typedef unsigned long long                 rep;
    typedef std::ratio<1, 4'000'000'000>       period; // My machine is 4.0 GHz
    typedef std::chrono::duration<rep, period> duration;
    typedef std::chrono::time_point<clock>     time_point;
    static const bool is_steady =              true;

    static time_point now() noexcept
    {
        unsigned lo, hi;
        return time_point(duration(static_cast<rep>(__rdtsc()));
    }
};
```

Obviously, real code couldn't hard-code a processor speed; this would need to be measured
or otherwise determined at runtime.

### QueryPerformanceCounter

QueryPerformanceCounter has a storied history. According to documentation, it uses either
the ACPI timer (also called PM clock), the newer HPET timer, or the TSC built into Intel
and AMD processors. By observation, it seems that QPC tends to use the ACPI timer.

On our Windows machine with a default install and settings, QPF returns 3914060. Oddly, this
isn't the ACPI timer rate; that's supposed to be 3579545 Hz.

The value returned by `QueryPerformanceFrequency` is guaranteed to not change after boot time;
theoretically it could change on each boot. So your program must fetch it at least once. It is
the number of `QueryPerformanceCounter` ticks per second.

```
LARGE_INTEGER freq;
QueryPerformanceFrequency(&freq);
double sec_per_qpc = 1.0 / freq.QuadPart;

LARGE_INTEGER start;
QueryPerformanceCounter(&start);
... code to measure ...
LARGE_INTEGER end;
QueryPerformanceCounter(&end);

double elapsed_seconds = (end - start) * sec_per_qpc;
```

A call to `QueryPerformanceCounter` is surprisingly fast, about 6 nanoseconds, at least when
using the ACPI timer method. This has a precision of about 4 Mhz, so it can't be used to
measure sub-500-nanosecond intervals.

Untested - there are reports that if you use the HPET timer, a single call to
`QueryPerformanceCounter` increases to about 500 nanoseconds, making the extra precision
pointless. This needs to be tested.

To force use of the HPET timer, set a boot param and reboot

```
bcdedit /set useplatformclock true
```

To restore back to the Windows default

```
bcdedit /deletevalue useplatformclock
```

It's interesting/frustrating that all the docs claim that QueryPerformanceCounter uses TSC if
"invariant TSC is available" but none of the machines I've tested on seem to satisfy that
constraint. Also interesting that documentation claims that the ACPI timer frequency is
3,579,545 Hz, which does not match the number I see from QPF. So perhaps there is some scaling
going on and QPC is really using the TSC?

When I call TSC and QPC side-by-side on this machine, each QPC tick is about 1030 RDTSC
ticks, which is consistent with a clock speed of 4 Ghz, which is the speed of this Core i7-6700K.
So I have to believe that QPC is really calling the ACPI timer, or some other timer that is
around that speed.

Processor’s support for invariant TSC is indicated by CPUID.80000007H:EDX[8]. I should check this.

## POSIX

`clock_gettime` with `CLOCK_MONOTONIC_RAW` is the preferred method. It directly returns seconds
and nanoseconds, but the precision is platform-dependent. On modern desktops, this is going to
be fairly fast, on the order of 150 nanoseconds.

```
double nano = 1.0 / 1000000000.0;

struct timespec start;
clock_gettime(CLOCK_MONOTONIC_RAW, &start);
... code being measured ...
struct timespec end;
clock_gettime(CLOCK_MONOTONIC_RAW, &end);

double elapsed_seconds = (end.tv_sec + end.tvnsec * nano) - (start.tv_sec + start.tvnsec * nano);
```

Unfortunately, `CLOCK_MONOTIC_RAW` is not available on some systems. Here's a list of clocks
to use in decreasing order of preference (these can be tested at compile time with #ifdef).

- `CLOCK_MONOTONIC_RAW`
- `CLOCK_MONOTONIC`
- `CLOCK_REALTIME`

Also note that POSIX has `clock_getres`, which returns the resolution (precision) of the
specified clock. Also note that calling this on many systems claims that the precision of
`CLOCK_MONOTONIC` is 1 nanosecond, and this is unlikely.

## Mac

`mach_absolute_time` because it doesn't have `clock_gettime`. Use `mach_timebase_info` to
determine the units. Since `mach_timebase_info` is actually telling us how many nanoseconds
are in a clock (as a rational number), we convert to seconds as part of turning the rational
number into a scalar multiplier.

```
mach_timebase_info_data_t info;
mach_timebase_info(&info);
double sec_per_clock = ((double) info.numer / info.denom) / 1000000000.0;

uint64_t start = mach_absolute_time();
... code being measured ...
uint64_t end = mach_absolute_time();

double elapsed_sec = (end - start) * sec_per_clock;
```

The `mach_timebase_info_data_t` is a rational number expressing nanoseconds per tick, similar
to what `<chrono>` does except with poor naming, only 32-bit for the rational components, 
nanoseconds instead of seconds, and not necessarily reduced to lowest terms.

On an iPad or iPhone 4, the underlying clock being sampled runs at 25 Mhz. On a Mac, the
timebase claims the clock runs at 1 Ghz, but this isn't credible.

Also note that if you're going to do the math with integers rather than floats, you'll want
to make sure the scaling on the `mach_timebase_info_data_t` members is either reduced as much
as possible, or that you use 128-bit multiplies and divides. Since our durations are expressed
as seconds, we want floating-point anyway.

## Linux

Just use the POSIX calls.

## BSD

Just use the POSIX calls.

# APIs

We typically care about time as two things

- a duration
- a point in time

We assume that a point in time is actually some duration before or after a predetermined epoch.

When we go to represent this as something concrete, we need to make some decisions. What are the units
of duration, and what is the epoch for a point in time?

In the current era, for times that are intended to be measurements of computer execution, it seems like
duration should be one of two things:

- 64-bit integer nanoseconds
- 64-bit floating number of seconds

With integer nanoseconds, we can specific any nanosecond within a 584 year span. For performance
measurement, this is several orders of magnitude more than we need. Even for dates attached to
computer-generated events, this is acceptable, since we had no such events before about 1940, and
this would take us to the year 2500. This isn't useful for general calendars, since we couldn't talk
about dates outside a 584 year range, no matter what the epoch. Subtracting and comparing 64-bit
integers is fast, which makes this appealing.

With 64-bit float seconds, we can cover a range larger than the lifespan of the universe. However, we
have a precision issue. We have a 52-bit mantissa - once we go past about a million seconds, we can
no longer represent every nanosecond. This doesn't mean that only durations longer than a million
seconds lose precision, it's how far we are from our epoch that determines the fuzziness. It gets
bad quickly.

- at 0.26 years from the epoch, we can only resolve to 1.8 nanoseconds
- at 4.2 years from the epoch, we can only resolve to 30 nanoseconds
- at 17 years from the epoch, we can only resolve to 119 nanoseconds
- at 34 years from the epoch, we can only resolve to 238 nanoseconds

Since our epoch is typically 1970, that means we are in the range of 238 nanosecond increments for
time points (the next doubling is in 2038).

Another thing to consider is that hardware is almost always a counter, so an integer. Obviously we
can't get more resolution than how fast the timer ticks. For modern desktop computers, this is usually
in the 4 Mhz range for an actual hardware timer, or in the Ghz range if it's a counter incremented by
the CPU itself.

The answer appears to be this - if you want a single duration value that covers both intervals and
arbitrary points in time, you need 64-bit floats, or you have a timing source in the low Mhz range,
use double. If you just want to measure elapsed time for the purpose of performance evaluation and you
have a fast timing source, then you should use 64-bit integer nanoseconds, as it's incredibly unlikely
you will want to measure events that are 538 years in duration.

Why not larger values? These cease to be easy to work with, requiring multiple machine instructions
to transfer or operate on. Certainly even a 80-bit float would have the range we wanted for nanosecond
timing, as it has 12 more bits of mantissa compared to 64-bit floats. Another approach would be Gustafson's Unum, but there is no hardware support for it yet.

You could make your epoch be machine boot time or program start time. You'll start to lose precision
as time passes, of course; after a single day, you're down to 29 picoseconds, and after a month you're
at 0.46 nanoseconds. Something to think about.

# Reference

[Acquiring high-resolution time stamps](https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx)

[USING HPET FOR A HIGH-RESOLUTION TIMER ON WINDOWS](http://blog.bfitz.us/?p=848)

[Microsecond Resolution Time Services for Windows](http://www.windowstimestamp.com/description)

[rdtsc in the Age of Sandybridge](https://randomascii.wordpress.com/2011/07/29/rdtsc-in-the-age-of-sandybridge/)

[QueryPerformanceCounter calibration with GetTickCount](https://www.janbambas.cz/queryperformancecounter-calibration-with-gettickcount/)

[Is QueryPerformanceFrequency accurate when using HPET?](https://stackoverflow.com/questions/22942123/is-queryperformancefrequency-accurate-when-using-hpet)

[High Resolution Clocks and Timers for Performance Measurement in Windows.](http://computerperformancebydesign.com/high-resolution-clocks-and-timers-for-performance-measurement-in-windows/)

[Mach Absolute Time Units](https://developer.apple.com/library/content/qa/qa1398/_index.html)

[How can I use mach_absolute_time without overflowing?](https://stackoverflow.com/questions/23378063/how-can-i-use-mach-absolute-time-without-overflowing)

[Timer function to provide time in nano seconds using C++](https://stackoverflow.com/questions/275004/timer-function-to-provide-time-in-nano-seconds-using-c/275231#275231)

[Posit Arithmetic](https://posithub.org/docs/Posits4.pdf)
