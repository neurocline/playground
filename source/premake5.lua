-- premake5.lua
--workspace "playground2"
--    configurations { "Debug", "Release" }
--    platforms { "x32", "x64" }

--project "playground2"
--    kind "ConsoleApp"
--    language "C++"
--    files { "**.cpp", "**.h" }

-- premake5.lua
workspace "playground3"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64" }

project "playground3"
    kind "ConsoleApp"
    language "C++"
    files { "source/**.cpp", "source/**.h" }
    
    filter "platforms:Win32"
        system "Windows"
        architecture "x86"
    filter "platforms:Win64"
        system "Windows"
        architecture "x86_64"
