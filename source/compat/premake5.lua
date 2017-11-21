local BUILD = "../../build/compat" -- we are two levels from the top

project("compat")
    location(BUILD)
    kind "ConsoleApp"
    --language "C++"
    --cppdialect "C++14"
    files { "**.cpp", "**.h"}
    --warnings "Extra"

    --filter { "toolset:msc*" }
    --    defines { "_ITERATOR_DEBUG_LEVEL=0", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
