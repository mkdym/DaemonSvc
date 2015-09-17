#pragma once
#include <string>
#include <iostream>



#if defined(_UNICODE) || defined(UNICODE)

#ifndef TEXT
#define TEXT(quote) L##quote
#endif

#define tcout std::wcout
typedef wchar_t tchar;
typedef std::wstring tstring;

#else

#ifndef TEXT
#define TEXT(quote) quote
#endif

#define tcout std::cout
typedef char tchar;
typedef std::string tstring;

#endif


