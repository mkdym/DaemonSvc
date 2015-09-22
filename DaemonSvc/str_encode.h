#pragma once
#include <string>



#if defined(_UNICODE) || defined(UNICODE)

#define tstr2widestr(tstr)              tstr
#define tstr2ansistr(tstr)              WideStr2ANSIStr(tstr)

#define widestr2tstr(wstr)              wstr
#define ansistr2tstr(str)               ANSIStr2WideStr(str)

#else

#define tstr2widestr(tstr)              ANSIStr2WideStr(tstr)
#define tstr2ansistr(tstr)              tstr

#define widestr2tstr(wstr)              WideStr2ANSIStr(wstr)
#define ansistr2tstr(str)               str

#endif



std::wstring ANSIStr2WideStr(const std::string& s);
std::string WideStr2ANSIStr(const std::wstring& ws);


