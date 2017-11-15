local BUILD = "../../build/bignum" -- we are two levels from the top

workspace "bignum"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location(BUILD)

project "bignum"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    -- flags { "C++14" }
    files { "main.cpp", "Bignum.cpp", "Bignum.h" }
    -- warnings "Extra"
    filter { "action:vs*" }
        -- buildoptions { '/std:c++17' }
        defines { "_ITERATOR_DEBUG_LEVEL=0", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
    filter { "action:xcode*" }
        buildoptions { '-std=c++17' }
