// ======================================================================================
// Win32App_simple.cpp
//
// A Win32 skeleton.
// ======================================================================================

#include "Win32App_simple.h"

#include <sstream>

WPARAM Win32App_simple::Execute()
{
    // Do program initialization. If we can't start up, return 0 per guidelines.
    int exitCode = 0;
    if (Startup())
    {
        // Enter the message loop, terminating when a WM_QUIT message is received.
        // We receive the wParam from the WM_QUIT message to use as our program return code.
        exitCode = MessageLoop();
        Shutdown();
    }

    return exitCode;
}

bool Win32App_simple::Startup()
{
    if (!RegisterWindowClasses())
        return false;

    if (!CreateWindows())
        return false;

    return true;
}

WPARAM Win32App_simple::MessageLoop()
{
    WPARAM exit_code = 0;

    // Run message loop until we get no more messages
    MSG msg;
    while (GetMessage (&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return exit_code;
}

void Win32App_simple::Shutdown()
{
}

LRESULT CALLBACK Win32App_simple::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    std::basic_ostringstream<Win32API::char_type> s;
    s << TEXT("message: ") << message << std::endl;
    OutputDebugStringW(s.str().c_str());

    switch (message)
    {
        case WM_CLOSE:
            OutputDebugString(TEXT("window closing\n"));
            break;
        case WM_DESTROY:
            OutputDebugString(TEXT("window being destroyed\n"));
            PostQuitMessage(0);
            return 0;
        // WM_QUIT is not sent to the WindowProc but swallowed by GetMessage?
        case WM_QUIT:
            OutputDebugString(TEXT("app exit\n"));
            break;
    }
    return ::DefWindowProc(hwnd, message, wParam, lParam);
}

bool Win32App_simple::CreateWindows()
{
    DWORD wstylesEx = 0;
    TCHAR* wclass = TEXT("Win32App_simple");
    TCHAR* wtitle = TEXT("Simple Win32App");
    DWORD wstyle = WS_OVERLAPPEDWINDOW;
    POINT wtopleft = {CW_USEDEFAULT, CW_USEDEFAULT};
    SIZE wdim = {0, 0};
    HWND wparent = NULL;
    HMENU wmenu = NULL;

    HWND wnd = CreateWindowEx(
        wstylesEx,
        wclass,
        wtitle,
        wstyle,
        wtopleft.x,
        wdim.cx,
        wtopleft.y,
        wdim.cy,
        wparent,
        wmenu,
        hInstance,
        NULL);
    if (wnd == NULL)
        return false;

    window = wnd;
    ShowWindow(window, nCmdShow);
    UpdateWindow(window);
    return true;
}

bool Win32App_simple::RegisterWindowClasses()
{
    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LPVOID);
    wc.hInstance = hInstance;
    wc.hIcon = Win32API::LoadIcon(hInstance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("Win32App_simple");
    wc.hIconSm = (HICON) LoadImage(hInstance, IDI_APPLICATION, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);;

    return RegisterClassEx(&wc) != FALSE;
}
