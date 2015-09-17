#pragma once
#include <string>
#include <time.h>
#include <Windows.h>
#include <boost/lexical_cast.hpp>
#include "logger.h"



static std::wstring ANSIStr2WideStr(const std::string& s)
{
    std::wstring ws;
    wchar_t *wide_str_buf = NULL;

    do 
    {
        int need_wchars = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size(), NULL, 0);
        if (0 == need_wchars)
        {
            break;
        }

        wide_str_buf = new wchar_t[need_wchars + 1];
        memset(wide_str_buf, 0, (need_wchars + 1) * sizeof(wchar_t));
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size(), wide_str_buf, need_wchars);
        if (0 == need_wchars)
        {
            DWORD e = GetLastError();
            break;
        }
        wide_str_buf[need_wchars] = L'\0';
        ws = wide_str_buf;

    } while (false);

    delete wide_str_buf;
    return ws;
}


static std::string LogPrefixA(const __LOG_LEVEL level, const char *file, const int line)
{
    std::string s;

    time_t raw_time = time(NULL);
    tm timeinfo = {0};
    if (0 == localtime_s(&timeinfo, &raw_time))
    {
        const int time_buffer_size = 100;
        char time_buffer[time_buffer_size] = {0};
        strftime(time_buffer, time_buffer_size, "%Y-%m-%d-%H:%M:%S", &timeinfo);
        time_buffer[time_buffer_size - 1] = 0;
        s += time_buffer;
    }
    else
    {
        s += "unknown time???";
    }
    s += " ";

    switch (level)
    {
    case LOG_DEBUG:
        s += "DEBUG";
        break;

    case LOG_INFO:
        s += "INFO ";
        break;

    case LOG_ERROR:
        s += "ERROR";
        break;

    default:
        s += "?????";
        break;
    }

    s += " [";
    s += file;
    s += ":" + boost::lexical_cast<std::string>(line) + "] ";
    return s;
}

static std::wstring LogPrefixW(const __LOG_LEVEL level, const char *file, const int line)
{
    std::string s = LogPrefixA(level, file, line);
    return ANSIStr2WideStr(s);
}


void __LogW(const __LOG_LEVEL level, const char *file, const int line, const wchar_t *format, ...)
{
    wchar_t buf[4096] = {0};
    va_list args;
    va_start(args, format);
    if (_vsnwprintf_s(buf, 4096, _TRUNCATE, format, args) < 0)
    {
        std::cout << "_vsnwprintf_s error" << std::endl;
    }
    va_end(args);
    buf[4096 - 1] = L'\0';

    std::wstring s = LogPrefixW(level, file, line) + buf;
    std::wcout << s.c_str() << std::endl;
    s += L"\r\n";
    OutputDebugStringW(s.c_str());
}

void __LogA(const __LOG_LEVEL level, const char *file, const int line, const char *format, ...)
{
    char buf[4096] = {0};
    va_list args;
    va_start(args, format);
    if (_vsnprintf_s(buf, 4096, _TRUNCATE, format, args) < 0)
    {
        std::cout << "_vsnprintf_s error" << std::endl;
    }
    va_end(args);
    buf[4096 - 1] = TEXT('\0');

    std::string s = LogPrefixA(level, file, line) + buf;
    std::cout << s.c_str() << std::endl;
    s += "\r\n";
    OutputDebugStringA(s.c_str());
}



