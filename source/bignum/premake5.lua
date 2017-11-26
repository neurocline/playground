local BUILD = "../../build/bignum" -- we are two levels from the top

project "bignum"
    location(BUILD)
    kind "ConsoleApp"
    --language "C++"
    --cppdialect "C++14"
    files { "**.cpp", "**.h", "../catch.hpp" }
    --warnings "Extra"

    --filter { "action:vs*" }
    --    -- toolset "msc-v140"
    --    defines { "_ITERATOR_DEBUG_LEVEL=0", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
    --filter { "action:xcode*" }
    --    buildoptions { '-std=c++1' }

    -- should this be done at the workspace level or the project level? If one library
    -- is using lzcnt intrinsics, we expect all of them to be compatible with that.
    -- is this a link issue?
    filter { "action:xcode*" }
      buildoptions { "-mlzcnt" }
