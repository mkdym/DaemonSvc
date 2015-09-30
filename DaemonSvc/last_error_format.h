#pragma once
#include <string>
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "vaformat.h"


//do not use any other user-defined function or class
//this is a base class
class CLastErrorFormat : public boost::noncopyable
{
public:
    CLastErrorFormat()
        : m_code(GetLastError())
    {
    }

    CLastErrorFormat(const DWORD code)
        : m_code(code)
    {
    }

    ~CLastErrorFormat()
    {
    }

public:
    const DWORD code() const
    {
        return m_code;
    }

    const std::string& str()
    {
        if (m_str.empty())
        {
            translate_error_code(m_code, m_str);
        }
        return m_str;
    }

    const std::wstring& wstr()
    {
        if (m_wstr.empty())
        {
            translate_error_code(m_code, m_wstr);
        }
        return m_wstr;
    }

private:
    //the two functions are implemented as same except CharType and API[A/W]
    //so, if you would modify one of them, modify the other then
    //I don't know how to merge them into one elegantly
    static bool translate_error_code(const DWORD code, std::string& s);
    static bool translate_error_code(const DWORD code, std::wstring& ws);

private:
    const DWORD m_code;
    std::string m_str;
    std::wstring m_wstr;
};


//max valist format string buffer
//you can change it
const size_t MAX_PRINT_LAST_ERROR_BUFFER = 4096;

#define print_last_err(e, s, ...)          _print_last_err(e, vaformat(MAX_PRINT_LAST_ERROR_BUFFER, s, __VA_ARGS__))


//do not use the functions which is started with "_", use macros
inline void _print_last_err(CLastErrorFormat& e, const std::string& prefix)
{
    //do not use std::cout, because it's not thread-safe
    printf_s("%s, error code: %lu, error msg: %s\r\n", prefix.c_str(), e.code(), e.str().c_str());
}

inline void _print_last_err(CLastErrorFormat& e, const std::wstring& wprefix)
{
    wprintf_s(L"%s, error code: %lu, error msg: %s\r\n", wprefix.c_str(), e.code(), e.wstr().c_str());
}




