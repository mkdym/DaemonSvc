#pragma once
#include <string>
#include <iostream>



#if defined(_UNICODE) || defined(UNICODE)
#define tcout std::wcout
typedef wchar_t tchar;
typedef std::wstring tstring;
#else
#define tcout std::cout
typedef char tchar;
typedef std::string tstring;
#endif



#if defined(_UNICODE) || defined(UNICODE)
#define ErrorLog        ErrorLogW
#define InfoLog         InfoLogW
#define DebugLog        DebugLogW
#else
#define ErrorLog        ErrorLogA
#define InfoLog         InfoLogA
#define DebugLog        DebugLogA
#endif


enum __LOG_LEVEL
{
    LOG_ERROR,
    LOG_INFO,
    LOG_DEBUG,
};


#define ErrorLog(format, ...)           __Log(LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define InfoLog(format, ...)            __Log(LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__)
#define DebugLog(format, ...)           __Log(LOG_DEBUG, __FILE__, __LINE__, format, __VA_ARGS__)

#define ErrorLogA(format, ...)          __LogA(LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define InfoLogA(format, ...)           __LogA(LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__)
#define DebugLogA(format, ...)          __LogA(LOG_DEBUG, __FILE__, __LINE__, format, __VA_ARGS__)

#define ErrorLogW(format, ...)          __LogW(LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define InfoLogW(format, ...)           __LogW(LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__)
#define DebugLogW(format, ...)          __LogW(LOG_DEBUG, __FILE__, __LINE__, format, __VA_ARGS__)


inline void __LogW(const __LOG_LEVEL level, const char *file, const int line, const wchar_t *format, ...)
{
    wchar_t buf[4096] = {0};
    va_list args;
    va_start(args, format);
    if (_vsnwprintf_s(buf, 4096, _TRUNCATE, format, args) < 0)
    {
        std::cout << "_vsnwprintf_s error" << std::endl;
    }
    va_end(args);
    buf[4096 - 1] = L'\0';

    std::wstring s = buf;
    std::wcout << s.c_str() << std::endl;
    s += L"\r\n";
    OutputDebugStringW(s.c_str());
}

inline void __LogA(const __LOG_LEVEL level, const char *file, const int line, const char *format, ...)
{
    char buf[4096] = {0};
    va_list args;
    va_start(args, format);
    if (_vsnprintf_s(buf, 4096, _TRUNCATE, format, args) < 0)
    {
        std::cout << "_vsnprintf_s error" << std::endl;
    }
    va_end(args);
    buf[4096 - 1] = TEXT('\0');

    std::string s = buf;
    std::cout << s.c_str() << std::endl;
    s += "\r\n";
    OutputDebugStringA(s.c_str());
}



