local BUILD = "../../build/timer" -- we are two levels from the top

workspace "timer"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location(BUILD)

project "timer"
    kind "ConsoleApp"
    language "C++"
    files { "main.cpp"}
    filter { "action:vs*" }
        buildoptions { '/std:c++17' }
    filter { "action:xcode*" }
        buildoptions { '-std=c++17' }
