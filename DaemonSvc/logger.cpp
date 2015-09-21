#pragma once
#include <string>
#include <time.h>
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr.hpp>
#include "str_encode.h"
#include "self_path.h"
#include "logger.h"



class __LogFile : public boost::noncopyable
{
public:
    static __LogFile& GetInstanceRef()
    {
        static __LogFile instance;
        return instance;
    }

private:
    __LogFile()
        : m_has_init(false)
        , m_hFile(INVALID_HANDLE_VALUE)
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
        if (m_has_init)
        {
            return true;
        }
        else
        {
            std::cout << "init log" << std::endl;
            bool bReturn = false;

            if (m_hFile != INVALID_HANDLE_VALUE)
            {
                CloseHandle(m_hFile);
                m_hFile = INVALID_HANDLE_VALUE;
            }

            do 
            {
                if (!CSelfPath::GetInstanceRef().init())
                {
                    std::cout << "can not get self path" << std::endl;
                    break;
                }

                tstring file_path = dir;
                if (file_path.empty())
                {
                    file_path = CSelfPath::GetInstanceRef().get_dir() + TSTR("\\log");
                    //file_path size limit 248, see MSDN CreateDirectory
                    if (!CreateDirectory(file_path.c_str(), NULL))
                    {
                        CLastError e;
                        if (ERROR_ALREADY_EXISTS != e.code())
                        {
                            print_last_err(e, TSTR("CreateDirectory for create log dir[%s] fail"), file_path.c_str());
                            break;
                        }
                    }
                }
                file_path += TSTR("\\") + CSelfPath::GetInstanceRef().get_name();

                {
                    time_t raw_time = time(NULL);
                    tm timeinfo = {0};
                    const int e = localtime_s(&timeinfo, &raw_time);
                    if (e)
                    {
                        std::cout << "localtime_s fail, return code: " << e << std::endl;
                        break;
                    }

                    const int time_buffer_size = 100;
                    tchar time_buffer[time_buffer_size] = {0};
                    _tcsftime(time_buffer, time_buffer_size, TSTR("%Y%m%d%H%M%S"), &timeinfo);
                    time_buffer[time_buffer_size - 1] = TSTR('\0');

                    file_path += TSTR(".");
                    file_path += time_buffer;
                }

                {
                    const DWORD pid = GetCurrentProcessId();
                    file_path += TSTR(".");
                    file_path += boost::lexical_cast<tstring>(pid);
                }

                file_path += TSTR(".log");
                m_hFile = CreateFile(file_path.c_str(),
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
                if (INVALID_HANDLE_VALUE == m_hFile)
                {
                    print_last_err(CLastError(), TSTR("CreateFile fail, file path: %s"), file_path.c_str());
                    break;
                }

                m_has_init = true;
                bReturn = true;

            } while (false);

            return bReturn;
        }
    }

    bool write(const void *buf, const DWORD len)
    {
        if (INVALID_HANDLE_VALUE == m_hFile)
        {
            std::cout << "must call log init first" << std::endl;
            return false;
        }
        else
        {
            DWORD written_bytes = 0;
            if (!WriteFile(m_hFile, buf, len, &written_bytes, NULL))
            {
                print_last_err(CLastError(), TSTR("WriteFile fail"));
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
    }

private:
    bool m_has_init;
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


#if defined(_UNICODE) || defined(UNICODE)
#define BuildPrefix         BuildPrefixW
#else
#define BuildPrefix         BuildPrefixA
#endif


/*
functions below are exported functions
*/

bool InitLog(const tstring& dir)
{
    return __LogFile::GetInstanceRef().init(dir);
}

bool __LogBytes(const __LOG_LEVEL level, const char *file, const int line,
                const tstring& prefix, const void *buf, const DWORD len)
{
    std::string s = BuildPrefixA(level, file, line)
        + tstr2ansistr(prefix) + "\r\n"
        + "@@@@@begin, buffer size = "
        + boost::lexical_cast<std::string>(len)
        + "@@@@@\r\n";
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
    const int count = _vsnprintf_s(buf, 4096, _TRUNCATE, format, args);
    if (count < 0)
    {
        std::cout << "_vsnprintf_s error" << std::endl;
    }
    va_end(args);

    std::string s = BuildPrefixA(level, file, line);
    s.append(buf, count);
    s += "\r\n";

    std::cout << s.c_str();
    OutputDebugStringA(s.c_str());

    __LogFile::GetInstanceRef().write(s.c_str(), s.size());
}

void __LogW(const __LOG_LEVEL level, const char *file, const int line, const wchar_t *format, ...)
{
    wchar_t buf[4096] = {0};
    va_list args;
    va_start(args, format);
    const int count = _vsnwprintf_s(buf, 4096, _TRUNCATE, format, args);
    if (count < 0)
    {
        std::cout << "_vsnwprintf_s error" << std::endl;
    }
    va_end(args);

    std::wstring ws = BuildPrefixW(level, file, line);
    ws.append(buf, count);
    ws += L"\r\n";

    std::wcout << ws.c_str();
    OutputDebugStringW(ws.c_str());

    const std::string s = WideStr2ANSIStr(ws);
    __LogFile::GetInstanceRef().write(s.c_str(), s.size());
}

void __LogLastErr(const __LOG_LEVEL level, const char *file, const int line, const CLastError& e, const tchar* prefix, ...)
{
    tchar buf[4096] = {0};
    va_list args;
    va_start(args, prefix);
    const int count = _vsntprintf_s(buf, 4096, _TRUNCATE, prefix, args);
    if (count < 0)
    {
        std::cout << "_vsntprintf_s error" << std::endl;
    }
    va_end(args);

    tstring err_str = BuildPrefix(level, file, line);
    err_str.append(buf, count);
    err_str += TSTR("\r\n");

    tcout << err_str.c_str();
    OutputDebugString(err_str.c_str());

    const std::string s = tstr2ansistr(err_str);
    __LogFile::GetInstanceRef().write(s.c_str(), s.size());
}




