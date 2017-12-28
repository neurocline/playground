// ======================================================================================
// Win32API.cpp
//
// A wrapped Win32 API that's a little more pleasant to call, using C++ idioms.
// ======================================================================================

#include "Win32API.h"

template <typename T>
auto basic_Win32API<T>::LoadString(UINT resID) -> typename basic_Win32API<T>::string_type
{
    const TCHAR* p = nullptr;
    int len;
    if constexpr(sizeof(TCHAR) == 2)
        len = ::LoadStringW(nullptr, resID, reinterpret_cast<LPWSTR>(&p), 0);
    else
        len = ::LoadStringA(nullptr, resID, reinterpret_cast<LPSTR>(&p), 0);
    if (len > 0)
        return string_type(p, static_cast<size_t>(len));

    // Return empty string; optionally replace with throwing an exception.
    return string_type();
}

template <class T>
HICON basic_Win32API<T>::LoadIcon(HINSTANCE hinst, LPCTSTR name)
{
    if constexpr(sizeof(TCHAR) == 2)
        return ::LoadIconW(hinst, (LPCWSTR) name);
    else
        return ::LoadIconA(hinst, (LPCSTR) name);
}

template <class T>
HICON basic_Win32API<T>::LoadIcon(HINSTANCE hinst, UINT resID)
{
    LPCTSTR name = MAKEINTRESOURCE(resID);
    return LoadIcon(hinst, name);
}

// Explicitly instantiate
template class basic_Win32API<TCHAR>;
