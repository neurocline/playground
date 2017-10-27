-- premake5.lua
workspace "test2"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64" }

project "test2"
    kind "ConsoleApp"
    language "C++"
    files { "source/**.cpp", "source/**.h" }
    
    filter "platforms:Win32"
        system "Windows"
        architecture "x86"
    filter "platforms:Win64"
        system "Windows"
        architecture "x86_64"
