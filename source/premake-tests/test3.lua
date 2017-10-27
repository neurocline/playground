local ROOT = "../../" -- we are two levels from the top
local PROJECT = ROOT .. "build/test3"

workspace "test3"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location(PROJECT)

project "test3"
    kind "ConsoleApp"
    language "C++"
    files { "**.cpp", "**.h" }
    objdir(PROJECT .. "/bin_%{cfg.platform}_%{cfg.buildcfg}/obj")
    targetdir(PROJECT .. "/bin_%{cfg.platform}_%{cfg.buildcfg}")
