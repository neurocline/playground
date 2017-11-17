-- premake5.lua
workspace "test1"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location("build")

project "test1"
    kind "ConsoleApp"
    language "C++"
    files { "source/**.cpp", "source/**.h" }
    
--    filter "platforms:Win32* or x32"
--        system "Windows"
--        architecture "x86"
--    filter "platforms:Win64* or x64"
--        system "Windows"
--        architecture "x86_64"

workspace "test2"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64" }
    location("build")

project "test2"
    kind "ConsoleApp"
    language "C++"
    files { "source/**.cpp", "source/**.h" }
    
--    filter "platforms:Win32* or x32"
--        system "Windows"
--        architecture "x86"
--    filter "platforms:Win64* or x64"
--        system "Windows"
--        architecture "x86_64"
