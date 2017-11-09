# Timers

## Windows

On Windows, QueryPerformanceCounter uses either the ACPI timer (also called PM clock), the newer
HPET timer, or the TSC built into Intel and AMD processors.

On a 2016-era motherboard with Windows 10 Pro version 1703 and default settings, QPF
returns 3914060 indicating use of the ACPI timer. Precision is 1 ACPI timer tick (255
nanoseconds), and call latency is under 6 nanoseconds. Documentation actually states that it
provides a time stamp in a single clock cycle.

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

Calling `rdtsc` in a tight loop gives an overhead of 24 clocks, or about 6 nanoseconds, similar
to the QPC loop, so it's likely that both have very low access time and most of that 6 nanoseconds
is the loop overhead.

### Reference

[Acquiring high-resolution time stamps](https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx)

[USING HPET FOR A HIGH-RESOLUTION TIMER ON WINDOWS](http://blog.bfitz.us/?p=848)

[Microsecond Resolution Time Services for Windows](http://www.windowstimestamp.com/description)

[rdtsc in the Age of Sandybridge](https://randomascii.wordpress.com/2011/07/29/rdtsc-in-the-age-of-sandybridge/)

[QueryPerformanceCounter calibration with GetTickCount](https://www.janbambas.cz/queryperformancecounter-calibration-with-gettickcount/)

[Is QueryPerformanceFrequency accurate when using HPET?](https://stackoverflow.com/questions/22942123/is-queryperformancefrequency-accurate-when-using-hpet)
