#pragma once
// ======================================================================================
// Win32App_simple.h
//
// A Win32 skeleton.
// ======================================================================================

#include "Win32API.h"

class Win32App_simple
{
public:
    Win32App_simple(HINSTANCE hinst, HINSTANCE hprev, LPSTR cmdline, int cmdShow):
        hInstance(hinst), hPrevInstance(hprev), lpCmdLine(cmdline), nCmdShow(cmdShow) {}

    WPARAM Execute();

    bool Startup();
    WPARAM MessageLoop();
    void Shutdown();

    bool RegisterWindowClasses();
    bool CreateWindows();

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    LPSTR lpCmdLine;
    int nCmdShow;

    HWND window;
};
