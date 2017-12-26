local BUILD = "../../build/win32app" -- we are two levels from the top

project "win32app"
    location(BUILD)
    kind "WindowedApp"
    entrypoint "WinMainCRTStartup" -- need this to get WinMainCRTStartup instead of mainCRTStartup

    files { "**.cpp", "**.h", "**.rc" }
    warnings "Extra"
