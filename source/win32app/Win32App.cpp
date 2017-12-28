// ======================================================================================
// Win32App.cpp
//
// A Win32 skeleton.
// ======================================================================================

#include "Win32App.h"

#include "resource.h"

#include <cassert>

#if 0
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    auto program = MakeWin32App();

    // Do program initialization. If we can't start up, return 0 per guidelines.
    int exitCode = 0;
    if (program->Startup(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
    {
        // Enter the message loop, terminating when a WM_QUIT message is received.
        // We receive the wParam from the WM_QUIT message to use as our program return code.
        exitCode = program->MessageLoop();

        program->Shutdown();
    }

    delete program;
    return exitCode;
}
#endif

bool Win32App::Startup(HINSTANCE hinst, HINSTANCE hprev, LPSTR cmdline, int cmdShow)
{
    hInstance = hinst;
    hPrevInstance = hprev;
    lpCmdLine = cmdline;
    nCmdShow = cmdShow;

    return RegisterWindowClasses();
};

LRESULT CALLBACK
MainWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return ::DefWindowProc(hwnd, message, wParam, lParam);
}

bool Win32App::RegisterWindowClasses()
{
    // Register a single window class for the main window
    Win32API::string_type ClassName = Win32API::LoadString(IDR_MAINWINDOW);
    auto icon = Win32API::LoadIcon(hInstance, IDR_MAINWINDOW);
    HICON small_icon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDR_MAINWINDOW), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
    auto menu = MAKEINTRESOURCE(IDR_MAINWINDOW);

    return RegisterWindowClass(ClassName, MainWndProc, menu, icon, small_icon);
}

bool Win32App::RegisterWindowClass(Win32API::string_type className, WNDPROC windProc, LPCTSTR menu, HICON icon, HICON small_icon)
{
    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = windProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = menu;
    wc.lpszClassName = className.c_str();
    wc.hIconSm = small_icon;

    return RegisterClassEx(&wc) != FALSE;
}

WPARAM Win32App::MessageLoop()
{
    // Load accelerator table for the main frame window's menu
    HACCEL haccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_MAINWINDOW));
    assert(haccel != NULL);

    WPARAM exit_code = 0;

    // Main message loop:
    MSG msg;
    while (GetMessage (&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, haccel, &msg))
        {
            TranslateMessage(&msg) ;
            DispatchMessage(&msg) ;
        }
    }

    return exit_code;
}

void Win32App::Shutdown()
{
}
