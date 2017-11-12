local BUILD = "../../build/timer" -- we are two levels from the top

workspace "timer"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location(BUILD)

project "timer"
    kind "ConsoleApp"
    language "C++"
    files { "main.cpp", "Timing.cpp", "Timing.h" }
    filter { "action:vs*" }
        buildoptions { '/std:c++17' }
        defines { "_ITERATOR_DEBUG_LEVEL=0", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
    filter { "action:xcode*" }
        buildoptions { '-std=c++17' }
