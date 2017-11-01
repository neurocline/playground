local ROOT = "../../" -- we are two levels from the top
local PROJECT = ROOT .. "build/test3"
local BIN = PROJECT .. "/bin_%{cfg.platform}_%{cfg.buildcfg}"

workspace "test3b"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location(PROJECT)

project "test3b"
    kind "ConsoleApp"
    language "C++"
    files { "**.cpp", "**.h" }
    objdir(BIN .. "/obj")
    targetdir(BIN)
