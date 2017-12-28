#pragma once
// ======================================================================================
// Win32API.h
//
// A wrapped Win32 API that's a little more pleasant to call, using C++ idioms.
// ======================================================================================

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// _WINDOWS is no longer defined or used anywhere
#if !defined(_WIN32)
#error this is a problem
#endif

// Remove <windows.h> macro pollution
#undef LoadString
#undef LoadIcon

#include <string>

// These are all the Win32 types that come in code page (A-suffix) and UTF-16LE (W-suffix)
// variants.
template <typename T>
class basic_Win32API
{
public:
    using string_type = std::basic_string<T>;

    // Calls LoadStringA or LoadStringW, returns appropriate std::string 
    static string_type LoadString(UINT resID);

    // Calls LoadIconA or LoadIconW
    static HICON LoadIcon(HINSTANCE hinst, LPCTSTR name);

    static HICON LoadIcon(HINSTANCE hinst, UINT resID);
};

// TBD - do UTF-8 conversion at this point and make the rest of the app code saner?

class Win32API : public basic_Win32API<TCHAR>
{
public:
};
