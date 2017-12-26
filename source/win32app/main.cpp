// main.cpp

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// _WINDOWS is no longer defined or used anywhere
#if !defined(_WIN32)
#error this is a problem
#endif

#define IDR_MAINFRAME 1
BOOL initInstance(HINSTANCE hinst, UINT resPoolID, int nCmdShow);

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE /*hPrevInstance*/,
    LPSTR /*lpCmdLine*/,
    int nCmdShow)
{
    if (!initInstance(hInstance, IDR_MAINFRAME, nCmdShow))
    {
        // If the function succeeds, terminating when it receives a WM_QUIT message, it should return
        // the exit value contained in that message's wParam parameter. If the function terminates
        // before entering the message loop, it should return zero.
        return 0;
    }
}

BOOL initInstance(HINSTANCE hinst, UINT resPoolID, int nCmdShow)
{
    return TRUE;
}

// echo VCTargetsPath=$(VCTargetsPath) && echo MSBuildToolsPath=$(MSBuildToolsPath) && echo MSBuildToolsPath32=$(MSBuildToolsPath32) && echo MSBuildToolsPath64=$(MSBuildToolsPath64) && echo FrameworkSDKRoot=$(FrameworkSDKRoot) && echo MSBuildSDKsPath=$(MSBuildSDKsPath) && echo VsInstallRoot=$(VsInstallRoot) && echo WindowsSDK_IncludePath=$(WindowsSDK_IncludePath)
