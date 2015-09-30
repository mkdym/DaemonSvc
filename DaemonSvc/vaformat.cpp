#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <boost/smart_ptr/scoped_array.hpp>
#include "vaformat.h"


static const size_t MIN_BUF_SIZE = 200;


//do not use Windows API, because Windows API will modify last error code
//while vaformat is always used to build last error log string
std::string vaformat(const size_t size_hint, const char* msg, ...)
{
    boost::scoped_array<char> scoped_buf;
    char buf_on_stack[MIN_BUF_SIZE] = {0};

    char* buf = buf_on_stack;
    size_t buf_size = MIN_BUF_SIZE;
    if (size_hint > MIN_BUF_SIZE)
    {
        //no need to memset, because _vsnprintf_s will return count
        scoped_buf.reset(new char[size_hint]);

        buf = scoped_buf.get();
        buf_size = size_hint;
    }

    std::string s;

    va_list args;
    va_start(args, msg);
    const int count = _vsnprintf_s(buf, buf_size, _TRUNCATE, msg, args);
    if (count < 0)
    {
        printf_s("_vsnprintf_s error\r\n");
    }
    else
    {
        //use append, rather than "=", in order to be correctly assigned when not null-terminated
        s.append(buf, count);
    }
    va_end(args);

    return s;
}

//see comments above
std::wstring vaformat(const size_t size_hint, const wchar_t* wmsg, ...)
{
    boost::scoped_array<wchar_t> scoped_buf;
    wchar_t buf_on_stack[MIN_BUF_SIZE] = {0};

    wchar_t* buf = buf_on_stack;
    size_t buf_size = MIN_BUF_SIZE;
    if (size_hint > MIN_BUF_SIZE)
    {
        //no need to memset, because _vsnprintf_s will return count
        scoped_buf.reset(new wchar_t[size_hint]);

        buf = scoped_buf.get();
        buf_size = size_hint;
    }

    std::wstring ws;

    va_list args;
    va_start(args, wmsg);
    const int count = _vsnwprintf_s(buf, buf_size, _TRUNCATE, wmsg, args);
    if (count < 0)
    {
        printf_s("_vsnwprintf_s error\r\n");
    }
    else
    {
        //use append, rather than "=", in order to be correctly assigned when not null-terminated
        ws.append(buf, count);
    }
    va_end(args);

    return ws;
}
