local ROOT = "../../build/" -- we are two levels from the top
local PROJECT = "test3c"
local WORKSPACE = ROOT .. PROJECT
local BIN = WORKSPACE .. "/bin_%{cfg.platform}_%{cfg.buildcfg}"

workspace(PROJECT)
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location(WORKSPACE)

project(PROJECT)
    kind "ConsoleApp"
    language "C++"
    files { "main.cpp" }
    objdir(BIN .. "/obj")
    targetdir(BIN)
