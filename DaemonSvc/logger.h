#pragma once
#include "common.h"



#if defined(_UNICODE) || defined(UNICODE)

#define ErrorLog        ErrorLogW
#define InfoLog         InfoLogW
#define DebugLog        DebugLogW

#define tstr2widestr(tstr)              tstr
#define tstr2ansistr(tstr)              WideStr2ANSIStr(tstr)

#else

#define ErrorLog        ErrorLogA
#define InfoLog         InfoLogA
#define DebugLog        DebugLogA

#define tstr2widestr(tstr)              ANSIStr2WideStr(tstr)
#define tstr2ansistr(tstr)              tstr

#endif


enum __LOG_LEVEL
{
    LOG_ERROR,
    LOG_INFO,
    LOG_DEBUG,
};

#define ErrorLogA(format, ...)          __LogA(LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define InfoLogA(format, ...)           __LogA(LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__)
#define DebugLogA(format, ...)          __LogA(LOG_DEBUG, __FILE__, __LINE__, format, __VA_ARGS__)

#define ErrorLogW(format, ...)          __LogW(LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define InfoLogW(format, ...)           __LogW(LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__)
#define DebugLogW(format, ...)          __LogW(LOG_DEBUG, __FILE__, __LINE__, format, __VA_ARGS__)


std::wstring ANSIStr2WideStr(const std::string& s);
std::string WideStr2ANSIStr(const std::wstring& ws);


bool InitLog(const tstring& dir);
bool LogBytes(const tstring& prefix, const void *buf, const DWORD len);
void __LogA(const __LOG_LEVEL level, const char *file, const int line, const char *format, ...);
void __LogW(const __LOG_LEVEL level, const char *file, const int line, const wchar_t *format, ...);



