// ======================================================================================
// main.cpp
//
// Our basic Win32 application
// ======================================================================================

#include "Win32API.h"
#include "Win32App.h"

#if 0

#include "resource.h"
BOOL initInstance(HINSTANCE hinst, UINT resPoolID, int nCmdShow);

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE /*hPrevInstance*/,
    LPSTR /*lpCmdLine*/,
    int nCmdShow)
{
    if (!initInstance(hInstance, IDS_STRING101, nCmdShow))
    {
        // If the function succeeds, terminating when it receives a WM_QUIT message, it should return
        // the exit value contained in that message's wParam parameter. If the function terminates
        // before entering the message loop, it should return zero.
        return 0;
    }

    return 0;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL initInstance(HINSTANCE hinst, UINT resPoolID, int nCmdShow)
{
    Win32API::string_type ClassName = Win32API::LoadString(resPoolID);
    auto icon = Win32API::LoadIcon(hinst, IDR_MAINWINDOW);
    HICON small_icon = (HICON) LoadImage(hinst, MAKEINTRESOURCE(IDR_MAINWINDOW), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);

    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hinst;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINWINDOW);
    wc.hIconSm = small_icon;

    return TRUE;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

#endif

Win32App* MakeWin32App() { return new Win32App; }
