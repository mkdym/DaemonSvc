#pragma once
#include <string>
#include <iostream>
#include <stdio.h>



#if defined(_UNICODE) || defined(UNICODE)

#ifndef TEXT
#define TEXT(quote) L##quote
#endif

#ifndef _tprintf_s
#define _tprintf_s wprintf_s
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

#define tcout std::cout
typedef char tchar;
typedef std::string tstring;

#endif


