-- premake5.lua
-- workspace configs may be overriden in project configs
workspace "playground"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location("../build")
    language "C++"
    cppdialect "C++17"
    warnings "Extra"

	filter { "platforms:*32" }
	    architecture "x86"
	filter { "platforms:*64" }
	    architecture "x64"
	filter { "toolset:msc*" }
	    defines { "_ITERATOR_DEBUG_LEVEL=0", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }

    -- should this be done at the workspace level or the project level? If one library
    -- is using lzcnt intrinsics, we expect all of them to be compatible with that.
    -- is this a link issue?
    filter { "action:xcode* or toolset:clang*" }
        buildoptions { "-mlzcnt" }

newoption {
   trigger     = "something",
   description = "Some description"
}

include "bignum"
include "compat"
include "file"
include "operators"
include "timer"
include "unicode"
include "win32app"
