#pragma once
#include <string>
#include <time.h>
#include <Windows.h>
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr.hpp>
#include "logger.h"



//do not log error
//because we use this function to log
std::wstring ANSIStr2WideStr(const std::string& s)
{
    assert(!s.empty());
    std::wstring ws;

    do 
    {
        int need_ch_len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size(), NULL, 0);
        if (0 == need_ch_len)
        {
            const DWORD e = GetLastError();
            std::cout << "MultiByteToWideChar fail when query need size, error code: " << e << std::endl;
            break;
        }

        boost::scoped_array<wchar_t> str_buf(new wchar_t[need_ch_len]);
        memset(str_buf.get(), 0, need_ch_len * sizeof(wchar_t));

        need_ch_len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size(), str_buf.get(), need_ch_len);
        if (0 == need_ch_len)
        {
            const DWORD e = GetLastError();
            std::cout << "MultiByteToWideChar fail, error code: " << e << std::endl;
            break;
        }

        ws.append(str_buf.get(), need_ch_len);

    } while (false);

    return ws;
}

//do not log error
//because we use this function to log
std::string WideStr2ANSIStr(const std::wstring& ws)
{
    assert(!ws.empty());
    std::string s;

    do 
    {
        int need_ch_len = WideCharToMultiByte(CP_ACP, 0, ws.c_str(), ws.size(), NULL, 0, NULL, NULL);
        if (0 == need_ch_len)
        {
            const DWORD e = GetLastError();
            std::cout << "WideCharToMultiByte fail when query need size, error code: " << e << std::endl;
            break;
        }

        boost::scoped_array<char> str_buf(new char[need_ch_len]);
        memset(str_buf.get(), 0, need_ch_len * sizeof(char));

        need_ch_len = WideCharToMultiByte(CP_ACP, 0, ws.c_str(), ws.size(), str_buf.get(), need_ch_len, NULL, NULL);
        if (0 == need_ch_len)
        {
            const DWORD e = GetLastError();
            std::cout << "WideCharToMultiByte fail, error code: " << e << std::endl;
            break;
        }

        s.append(str_buf.get(), need_ch_len);

    } while (false);

    return s;
}



class __LogFile
{
public:
    static __LogFile& GetInstanceRef()
    {
        static __LogFile instance;
        return instance;
    }

private:
    __LogFile()
        : m_hFile(INVALID_HANDLE_VALUE)
    {
    }

    ~__LogFile()
    {
        if (m_hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_hFile);
            m_hFile = INVALID_HANDLE_VALUE;
        }
    }

public:
    bool init(const tstring& dir)
    {
        if (m_hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_hFile);
            m_hFile = INVALID_HANDLE_VALUE;
        }

        bool bReturn = false;

        do 
        {
            tstring file_path = dir + TEXT("\\");

            {
                const DWORD len = 4096;
                tchar binary_file[len] = {0};
                if (!GetModuleFileName(NULL, binary_file, len - 1))
                {
                    const DWORD e = GetLastError();
                    std::cout << "GetModuleBaseName fail, error code: " << e << std::endl;
                    break;
                }

#if !defined(_tsplitpath_s)
#if defined(_UNICODE) || defined(UNICODE)
#define _tsplitpath_s _wsplitpath_s
#else
#define _tsplitpath_s _splitpath_s
#endif
#endif
                const DWORD FNAME_LEN = 1000;
                tchar name_buf[FNAME_LEN] = {0};
                _tsplitpath_s(binary_file, NULL, 0, NULL, 0, name_buf, FNAME_LEN, NULL, 0);
                name_buf[FNAME_LEN - 1] = TEXT('\0');

                file_path += name_buf;
            }

            {
                time_t raw_time = time(NULL);
                tm timeinfo = {0};
                const int e = localtime_s(&timeinfo, &raw_time);
                if (e)
                {
                    std::cout << "localtime_s fail, return code: " << e << std::endl;
                    break;
                }

#if !defined(_tcsftime)
#if defined(_UNICODE) || defined(UNICODE)
#define _tcsftime wcsftime
#else
#define _tcsftime strftime
#endif
#endif

                const int time_buffer_size = 100;
                tchar time_buffer[time_buffer_size] = {0};
                _tcsftime(time_buffer, time_buffer_size, TEXT("%Y%m%d%H%M%S"), &timeinfo);
                time_buffer[time_buffer_size - 1] = TEXT('\0');

                file_path += TEXT(".");
                file_path += time_buffer;
            }

            {
                const DWORD pid = GetCurrentProcessId();
                file_path += TEXT(".");
                file_path += boost::lexical_cast<tstring>(pid);
            }

            file_path += TEXT(".log");
            m_hFile = CreateFile(file_path.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            if (INVALID_HANDLE_VALUE == m_hFile)
            {
                const DWORD e = GetLastError();
                tcout << TEXT("CreateFile fail, file path: ") << file_path.c_str() << TEXT(", error code: ") << e << std::endl;
                break;
            }

            bReturn = true;

        } while (false);

        return bReturn;
    }

    bool write(const void *buf, const DWORD len)
    {
        DWORD written_bytes = 0;
        if (!WriteFile(m_hFile, buf, len, &written_bytes, NULL))
        {
            const DWORD e = GetLastError();
            std::cout << "WriteFile fail, error code: " << e << std::endl;
            return false;
        }
        else
        {
            if (written_bytes != len)
            {
                std::cout << "not all written, to write: " << len << ", written: " << written_bytes << std::endl;
            }
            return true;
        }
    }

private:
    HANDLE m_hFile;
};


static std::string BuildPrefixA(const __LOG_LEVEL level, const char *file, const int line)
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

    static const DWORD pid = GetCurrentProcessId();
    s += "[" + boost::lexical_cast<std::string>(pid) + ":" + boost::lexical_cast<std::string>(GetCurrentThreadId()) + "] ";

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

static std::wstring BuildPrefixW(const __LOG_LEVEL level, const char *file, const int line)
{
    std::string s = BuildPrefixA(level, file, line);
    return ANSIStr2WideStr(s);
}


/*
functions below are exported functions
*/

bool InitLog(const tstring& dir)
{
    return __LogFile::GetInstanceRef().init(dir);
}

bool LogBytes(const tstring& prefix, const void *buf, const DWORD len)
{
    std::string s = tstr2ansistr(prefix) + "\r\n" + "@@@@@begin, buffer size = " + boost::lexical_cast<std::string>(len) + "@@@@@\r\n";
    s.append(reinterpret_cast<const char *>(buf), len);
    s += "\r\n";
    s += "@@@@@end, buffer size = " + boost::lexical_cast<std::string>(len) + "@@@@@\r\n";

    return __LogFile::GetInstanceRef().write(s.c_str(), s.size());
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

    std::string s = BuildPrefixA(level, file, line) + buf + "\r\n";
    std::cout << s.c_str();
    OutputDebugStringA(s.c_str());

    __LogFile::GetInstanceRef().write(s.c_str(), s.size());
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

    std::wstring ws = BuildPrefixW(level, file, line) + buf + L"\r\n";
    std::wcout << ws.c_str();
    OutputDebugStringW(ws.c_str());

    const std::string s = WideStr2ANSIStr(ws);
    __LogFile::GetInstanceRef().write(s.c_str(), s.size());
}



