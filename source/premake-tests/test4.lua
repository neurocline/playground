local BUILD = "../../build/test4" -- we are two levels from the top

workspace "test4"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location(BUILD)

project "cmd"
    kind "ConsoleApp"
    language "C++"
    files { "main2.cpp"}
    links { "compute" }

project "compute"
    kind "StaticLib"
    language "C++"
    files { "compute.cpp"}
