#pragma once
#include <string>


//max_size is the max size of string size
//TRUNCATE if too small
//if max_size <= MIN_BUF_SIZE, will be MIN_BUF_SIZE, and use stack buffer, or will alloc heap buffer
//currently, MIN_BUF_SIZE = 1024
std::string vaformat(const size_t max_size, const char* msg, ...);
std::wstring vaformat(const size_t max_size, const wchar_t* wmsg, ...);



