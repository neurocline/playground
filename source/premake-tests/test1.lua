-- premake5.lua
workspace "test1"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }

project "test1"
    kind "ConsoleApp"
    language "C++"
    files { "**.cpp", "**.h" }
