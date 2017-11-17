-- premake5.lua
workspace "playground6"
    configurations { "Debug", "Release" }
    platforms { "Win32Fast", "Win64Slow" }

project "playground6"
    kind "ConsoleApp"
    language "C++"
    files { "source/**.cpp", "source/**.h" }
    
    filter "platforms:Win32*"
        system "Windows"
        architecture "x86"
    filter "platforms:Win64*"
        system "Windows"
        architecture "x86_64"
