-- premake5.lua

local BUILD = "../../build/compat" -- we are two levels from the top
local WORKSPACE = "compat"

workspace(WORKSPACE)
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location(BUILD)
    -- toolset "msc-v140"

project(WORKSPACE)
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    files { "**.cpp", "**.h"}
    warnings "Extra"

    filter { "toolset:msc*" }
        defines { "_ITERATOR_DEBUG_LEVEL=0", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
