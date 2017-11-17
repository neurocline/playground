local BUILD = "../../build/bignum" -- we are two levels from the top

project "bignum"
    location(BUILD)
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    files { "main.cpp", "Bignum.cpp", "Bignum.h" }
    warnings "Extra"

    filter { "action:vs*" }
        -- toolset "msc-v140"
        defines { "_ITERATOR_DEBUG_LEVEL=0", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
    filter { "action:xcode*" }
        buildoptions { '-std=c++17' }
