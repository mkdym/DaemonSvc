#pragma once
#include "tdef.h"
#include "last_error.h"



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

#define ErrorLogBytes(prefix, buf, len)         __LogBytes(LOG_ERROR, __FILE__, __LINE__, prefix, buf, len)
#define InfoLogBytes(prefix, buf, len)          __LogBytes(LOG_INFO, __FILE__, __LINE__, prefix, buf, len)
#define DebugLogBytes(prefix, buf, len)         __LogBytes(LOG_DEBUG, __FILE__, __LINE__, prefix, buf, len)

#define ErrorLogA(format, ...)          __LogA(LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define InfoLogA(format, ...)           __LogA(LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__)
#define DebugLogA(format, ...)          __LogA(LOG_DEBUG, __FILE__, __LINE__, format, __VA_ARGS__)

#define ErrorLogW(format, ...)          __LogW(LOG_ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
#define InfoLogW(format, ...)           __LogW(LOG_INFO, __FILE__, __LINE__, format, __VA_ARGS__)
#define DebugLogW(format, ...)          __LogW(LOG_DEBUG, __FILE__, __LINE__, format, __VA_ARGS__)

#define ErrorLogLastErr(e, prefix, ...)         __LogLastErr(LOG_ERROR, __FILE__, __LINE__, e, prefix, __VA_ARGS__)
#define InfoLogLastErr(e, prefix, ...)          __LogLastErr(LOG_INFO, __FILE__, __LINE__, e, prefix, __VA_ARGS__)
#define DebugLogLastErr(e, prefix, ...)         __LogLastErr(LOG_DEBUG, __FILE__, __LINE__, e, prefix, __VA_ARGS__)


bool InitLog(const tstring& dir);

bool __LogBytes(const __LOG_LEVEL level, const char *file, const int line,
                const tstring& prefix, const void *buf, const DWORD len);

void __LogA(const __LOG_LEVEL level, const char *file, const int line, const char *format, ...);
void __LogW(const __LOG_LEVEL level, const char *file, const int line, const wchar_t *format, ...);

void __LogLastErr(const __LOG_LEVEL level, const char *file, const int line, const CLastError& e, const tchar* prefix, ...);



