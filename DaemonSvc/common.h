#pragma once
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>



#if defined(_UNICODE) || defined(UNICODE)

#ifndef TEXT
#define TEXT(quote) L##quote
#endif

#ifndef _tprintf_s
#define _tprintf_s wprintf_s
#endif

#ifndef _vsntprintf_s
#define _vsntprintf_s _vsnwprintf_s
#endif

#ifndef _tsplitpath_s
#define _tsplitpath_s _wsplitpath_s
#endif

#ifndef _tcsftime
#define _tcsftime wcsftime
#endif

#define tcout std::wcout
typedef wchar_t tchar;
typedef std::wstring tstring;

#else

#ifndef TEXT
#define TEXT(quote) quote
#endif

#ifndef _tprintf_s
#define _tprintf_s printf_s
#endif

#ifndef _vsntprintf_s
#define _vsntprintf_s _vsnprintf_s
#endif

#ifndef _tsplitpath_s
#define _tsplitpath_s _splitpath_s
#endif

#ifndef _tcsftime
#define _tcsftime strftime
#endif

#define tcout std::cout
typedef char tchar;
typedef std::string tstring;

#endif


