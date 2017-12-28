#pragma once
// ======================================================================================
// Win32App.h
//
// A Win32 skeleton.
// ======================================================================================

#include "Win32API.h"

class Win32App
{
public:
    virtual bool Startup(HINSTANCE hinst, HINSTANCE hprev, LPSTR cmdline, int cmdShow);
    virtual WPARAM MessageLoop();
    virtual void Shutdown();

    virtual bool RegisterWindowClasses();
    virtual bool RegisterWindowClass(Win32API::string_type className, WNDPROC windProc, LPCTSTR menu, HICON icon, HICON small_icon);


    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    LPSTR lpCmdLine;
    int nCmdShow;
};

Win32App* MakeWin32App(); // defined in user code
