#pragma once
#include <string>



#if defined(_UNICODE) || defined(UNICODE)

#define tstr2widestr(tstr)              tstr
#define tstr2ansistr(tstr)              widestr2ansistr(tstr)

#define widestr2tstr(wstr)              wstr
#define ansistr2tstr(str)               ansistr2widestr(str)

#else

#define tstr2widestr(tstr)              ANSIStr2WideStr(tstr)
#define tstr2ansistr(tstr)              tstr

#define widestr2tstr(wstr)              WideStr2ANSIStr(wstr)
#define ansistr2tstr(str)               str

#endif



std::wstring ansistr2widestr(const std::string& s);
std::string widestr2ansistr(const std::wstring& ws);


