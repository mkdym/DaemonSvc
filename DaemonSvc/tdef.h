#pragma once
#include <string>
//#include <iostream>
//#include <stdio.h>
//#include <stdarg.h>



#if defined(_UNICODE) || defined(UNICODE)

#ifndef TSTR
#define TSTR(quote) L##quote
#endif

//#ifndef _tprintf_s
//#define _tprintf_s wprintf_s
//#endif
//
//#ifndef _stprintf_s
//#define _stprintf_s swprintf_s
//#endif
//
//#ifndef _vsntprintf_s
//#define _vsntprintf_s _vsnwprintf_s
//#endif
//
//#ifndef _tsplitpath_s
//#define _tsplitpath_s _wsplitpath_s
//#endif

//#define tcout std::wcout
typedef wchar_t tchar;
typedef std::wstring tstring;

#else

#ifndef TSTR
#define TSTR(quote) quote
#endif

//#ifndef _tprintf_s
//#define _tprintf_s printf_s
//#endif
//
//#ifndef _stprintf_s
//#define _stprintf_s sprintf_s
//#endif
//
//#ifndef _vsntprintf_s
//#define _vsntprintf_s _vsnprintf_s
//#endif
//
//#ifndef _tsplitpath_s
//#define _tsplitpath_s _splitpath_s
//#endif
//
//#define tcout std::cout
typedef char tchar;
typedef std::string tstring;

#endif


