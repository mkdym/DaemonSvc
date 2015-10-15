#pragma once
#include <string>



#if defined(_UNICODE) || defined(UNICODE)

#define tstr2widestr(tstr)              tstr
#define tstr2ansistr(tstr)              widestr2ansistr(tstr)

#define widestr2tstr(wstr)              wstr
#define ansistr2tstr(str)               ansistr2widestr(str)

#else

#define tstr2widestr(tstr)              ansistr2widestr(tstr)
#define tstr2ansistr(tstr)              tstr

#define widestr2tstr(wstr)              widestr2ansistr(wstr)
#define ansistr2tstr(str)               str

#endif



//ansi-string: the current system Windows ANSI code page string. typically, gb2312 on Simplified-Chinese Windows
//the following two functions are just implemented by multistr2widestr and widestr2multistr with code_page CP_ACP
std::wstring ansistr2widestr(const std::string& s);
std::string widestr2ansistr(const std::wstring& ws);


//implemented by Windows API MultiByteToWideChar
//if code_page is invalid, it will return empty string
std::wstring multistr2widestr(const unsigned int from_code_page, const std::string& s);
//default_char: when can not translate, system will use default char
//if NULL == default_char, use system default default char(maybe is "?")
//see Windows API WideCharToMultiByte
std::string widestr2multistr(const unsigned int to_code_page, const std::wstring& ws, const char *default_char = NULL);


