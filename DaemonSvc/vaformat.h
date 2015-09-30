#pragma once
#include <string>


//size_hint is the max size of string size
//TRUNCATE if too small
//if size_hint < MIN_BUF_SIZE, will be MIN_BUF_SIZE
//currently, MIN_BUF_SIZE = 200
std::string vaformat(const size_t size_hint, const char* msg, ...);
std::wstring vaformat(const size_t size_hint, const wchar_t* wmsg, ...);



