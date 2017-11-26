local BUILD = "../../build/operators" -- we are two levels from the top

project "operators"
    location(BUILD)
    kind "ConsoleApp"
    files { "**.cpp", "**.h", "../catch.hpp" }
