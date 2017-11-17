-- premake5.lua
workspace "playground"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location("../build")

filter { "platforms:x32" }
    architecture "x86"
filter { "platforms:x64" }
    architecture "x64"

include "bignum"
include "compat"
include "file"
include "timer"
