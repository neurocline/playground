local BUILD = "../../build/buffer" -- we are two levels from the top

project "buffer"
    location(BUILD)
    kind "ConsoleApp"
    files { "**.cpp", "**.h", "../catch.hpp" }

    -- should this be done at the workspace level or the project level? If one library
    -- is using lzcnt intrinsics, we expect all of them to be compatible with that.
    -- is this a link issue?
    filter { "action:xcode*" }
      buildoptions { "-mlzcnt" }
