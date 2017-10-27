local ROOT = "../../" -- we are two levels from the top

workspace "test2"
    location(ROOT .. "build/test2")
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }

project "test2"
    kind "ConsoleApp"
    language "C++"
    files { "**.cpp", "**.h" }
