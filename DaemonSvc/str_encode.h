#pragma once
#include <string>



#if defined(_UNICODE) || defined(UNICODE)

#define tstr2widestr(tstr)              tstr
#define tstr2ansistr(tstr)              WideStr2ANSIStr(tstr)

#else

#define tstr2widestr(tstr)              ANSIStr2WideStr(tstr)
#define tstr2ansistr(tstr)              tstr

#endif



std::wstring ANSIStr2WideStr(const std::string& s);
std::string WideStr2ANSIStr(const std::wstring& ws);


