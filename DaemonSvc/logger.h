#pragma once
#include "common.h"



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

#define ErrorLogA(format, ...)          __LogA(LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define InfoLogA(format, ...)           __LogA(LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__)
#define DebugLogA(format, ...)          __LogA(LOG_DEBUG, __FILE__, __LINE__, format, __VA_ARGS__)

#define ErrorLogW(format, ...)          __LogW(LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define InfoLogW(format, ...)           __LogW(LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__)
#define DebugLogW(format, ...)          __LogW(LOG_DEBUG, __FILE__, __LINE__, format, __VA_ARGS__)

void __LogW(const __LOG_LEVEL level, const char *file, const int line, const wchar_t *format, ...);
void __LogA(const __LOG_LEVEL level, const char *file, const int line, const char *format, ...);



