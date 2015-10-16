#pragma once
#include <Windows.h>
#include <boost/smart_ptr/scoped_array.hpp>
#include "last_error_format.h"
#include "str_encode.h"


//!!!
//all export functions in this file must not use log
//because we use these functions to log


static const unsigned int CP_US_ASCII = 20127;



std::wstring multistr2widestr(const unsigned int from_code_page, const std::string& s)
{
    std::wstring ws;

    do 
    {
        if (s.empty())
        {
            break;
        }

        int need_ch_len = MultiByteToWideChar(from_code_page, 0, s.c_str(), s.size(), NULL, 0);
        if (0 == need_ch_len)
        {
            print_last_err("MultiByteToWideChar fail when query need size");
            break;
        }

        boost::scoped_array<wchar_t> str_buf(new wchar_t[need_ch_len]);
        memset(str_buf.get(), 0, need_ch_len * sizeof(wchar_t));

        need_ch_len = MultiByteToWideChar(from_code_page, 0, s.c_str(), s.size(), str_buf.get(), need_ch_len);
        if (0 == need_ch_len)
        {
            print_last_err("MultiByteToWideChar fail");
            break;
        }

        ws.append(str_buf.get(), need_ch_len);

    } while (false);

    return ws;
}

std::string widestr2multistr(const unsigned int to_code_page, const std::wstring& ws, const char *default_char /*= NULL*/)
{
    std::string s;

    do 
    {
        if (ws.empty())
        {
            break;
        }

        //lpUsedDefaultChar: For the CP_UTF7 and CP_UTF8 settings for CodePage, this parameter must be set to a null pointer
        //Otherwise, the function fails with ERROR_INVALID_PARAMETER.
        //see MSDN
        const char *final_default_char = default_char;
        if (to_code_page == CP_UTF7
            || to_code_page == CP_UTF8)
        {
            final_default_char = NULL;
        }

        int need_ch_len = WideCharToMultiByte(to_code_page, 0, ws.c_str(), ws.size(), NULL, 0, final_default_char, NULL);
        if (0 == need_ch_len)
        {
            print_last_err("WideCharToMultiByte fail when query need size");
            break;
        }

        boost::scoped_array<char> str_buf(new char[need_ch_len]);
        memset(str_buf.get(), 0, need_ch_len * sizeof(char));

        need_ch_len = WideCharToMultiByte(to_code_page, 0, ws.c_str(), ws.size(), str_buf.get(), need_ch_len, final_default_char, NULL);
        if (0 == need_ch_len)
        {
            print_last_err("WideCharToMultiByte fail");
            break;
        }

        s.append(str_buf.get(), need_ch_len);

    } while (false);

    return s;
}


