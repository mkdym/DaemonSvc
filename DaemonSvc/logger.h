#pragma once
#include <string>
#include "log_level.h"
#include "vaformat.h"
#include "last_error_format.h"


//you should call InitLog first
//do not use the functions which is started with "_", use macros
//use char version as possible as you can
//because wchar_t version is based on char version: all wchar_t string will be converted to char string


//max valist format string buffer
//you can change it
const size_t MAX_LOG_BUFFER = 4096;



#define ErrorLog(s, ...)          _Log(LOG_ERROR, __FILE__, __LINE__, vaformat(MAX_LOG_BUFFER, s, __VA_ARGS__))
#define InfoLog(s, ...)           _Log(LOG_INFO, __FILE__, __LINE__, vaformat(MAX_LOG_BUFFER, s, __VA_ARGS__))
#define DebugLog(s, ...)          _Log(LOG_DEBUG, __FILE__, __LINE__, vaformat(MAX_LOG_BUFFER, s, __VA_ARGS__))

#define ErrorLogBytes(buf, len, prefix, ...)         \
    _LogBytes(LOG_ERROR, __FILE__, __LINE__, buf, len, vaformat(MAX_LOG_BUFFER, prefix, __VA_ARGS__))
#define InfoLogBytes(buf, len, prefix, ...)          \
    _LogBytes(LOG_INFO, __FILE__, __LINE__, buf, len, vaformat(MAX_LOG_BUFFER, prefix, __VA_ARGS__))
#define DebugLogBytes(buf, len, prefix, ...)         \
    _LogBytes(LOG_DEBUG, __FILE__, __LINE__, buf, len, vaformat(MAX_LOG_BUFFER, prefix, __VA_ARGS__))

#define ErrorLogLastErr(e, prefix, ...)         \
    _LogLastErr(LOG_ERROR, __FILE__, __LINE__, e, vaformat(MAX_LOG_BUFFER, prefix, __VA_ARGS__))
#define InfoLogLastErr(e, prefix, ...)          \
    _LogLastErr(LOG_INFO, __FILE__, __LINE__, e, vaformat(MAX_LOG_BUFFER, prefix, __VA_ARGS__))
#define DebugLogLastErr(e, prefix, ...)         \
    _LogLastErr(LOG_DEBUG, __FILE__, __LINE__, e, vaformat(MAX_LOG_BUFFER, prefix, __VA_ARGS__))


bool InitLog(const std::string& dir, const unsigned long max_size);
bool InitLog(const std::wstring& dir, const unsigned long max_size);


bool _Log(const LOG_LEVEL level, const char *file, const int line, const std::string& s);
bool _Log(const LOG_LEVEL level, const char *file, const int line, const std::wstring& ws);

bool _LogBytes(const LOG_LEVEL level, const char *file, const int line,
               const void *buf, const unsigned long len, const std::string& prefix);
bool _LogBytes(const LOG_LEVEL level, const char *file, const int line,
               const void *buf, const unsigned long len, const std::wstring& wprefix);

bool _LogLastErr(const LOG_LEVEL level, const char *file, const int line,
                 CLastErrorFormat& e, const std::string& prefix);
bool _LogLastErr(const LOG_LEVEL level, const char *file, const int line,
                 CLastErrorFormat& e, const std::wstring& wprefix);




