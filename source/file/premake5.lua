local BUILD = "../../build/file" -- we are two levels from the top

project "file"
    location(BUILD)
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    files { "main.cpp", "timer.h" }
    warnings "Extra"

    filter { "toolset:msc*" }
        defines { "_ITERATOR_DEBUG_LEVEL=0", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
