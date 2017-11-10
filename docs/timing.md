# Timing

Optimizing code for performance requires that you measure it, and carefully, because many
optimizations are counterintuitive.

The overhead for all performance measurement calls varies by operating system and hardware.
In the following, we have

- Windows: Core i7-6700K at 4GHZ running Windows 10 Pro version 1703, Visual Studio 2017

## C++11

C++11 added the `<chrono>` class as a cross-platform way to work with time in a generic
but performant way.

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
scalar of seconds turns that into about 55 nanoseconds.

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

## Linux

Just use the POSIX calls.

### Reference

[Acquiring high-resolution time stamps](https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx)

[USING HPET FOR A HIGH-RESOLUTION TIMER ON WINDOWS](http://blog.bfitz.us/?p=848)

[Microsecond Resolution Time Services for Windows](http://www.windowstimestamp.com/description)

[rdtsc in the Age of Sandybridge](https://randomascii.wordpress.com/2011/07/29/rdtsc-in-the-age-of-sandybridge/)

[QueryPerformanceCounter calibration with GetTickCount](https://www.janbambas.cz/queryperformancecounter-calibration-with-gettickcount/)

[Is QueryPerformanceFrequency accurate when using HPET?](https://stackoverflow.com/questions/22942123/is-queryperformancefrequency-accurate-when-using-hpet)

[High Resolution Clocks and Timers for Performance Measurement in Windows.](http://computerperformancebydesign.com/high-resolution-clocks-and-timers-for-performance-measurement-in-windows/)
