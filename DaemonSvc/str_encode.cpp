#pragma once
#include <Windows.h>
#include <boost/smart_ptr/scoped_array.hpp>
#include "last_error_format.h"
#include "str_encode.h"


//do not use log
//because we use this function to log
std::wstring ansistr2widestr(const std::string& s)
{
    std::wstring ws;

    do 
    {
        if (s.empty())
        {
            break;
        }

        int need_ch_len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size(), NULL, 0);
        if (0 == need_ch_len)
        {
            print_last_err("MultiByteToWideChar fail when query need size");
            break;
        }

        boost::scoped_array<wchar_t> str_buf(new wchar_t[need_ch_len]);
        memset(str_buf.get(), 0, need_ch_len * sizeof(wchar_t));

        need_ch_len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size(), str_buf.get(), need_ch_len);
        if (0 == need_ch_len)
        {
            print_last_err("MultiByteToWideChar fail");
            break;
        }

        ws.append(str_buf.get(), need_ch_len);

    } while (false);

    return ws;
}

//do not use log
//because we use this function to log
std::string widestr2ansistr(const std::wstring& ws)
{
    std::string s;

    do 
    {
        if (ws.empty())
        {
            break;
        }

        int need_ch_len = WideCharToMultiByte(CP_ACP, 0, ws.c_str(), ws.size(), NULL, 0, NULL, NULL);
        if (0 == need_ch_len)
        {
            print_last_err("WideCharToMultiByte fail when query need size");
            break;
        }

        boost::scoped_array<char> str_buf(new char[need_ch_len]);
        memset(str_buf.get(), 0, need_ch_len * sizeof(char));

        need_ch_len = WideCharToMultiByte(CP_ACP, 0, ws.c_str(), ws.size(), str_buf.get(), need_ch_len, NULL, NULL);
        if (0 == need_ch_len)
        {
            print_last_err("WideCharToMultiByte fail");
            break;
        }

        s.append(str_buf.get(), need_ch_len);

    } while (false);

    return s;
}


