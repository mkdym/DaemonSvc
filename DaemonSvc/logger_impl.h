#pragma once
#include <string>
#include <boost/thread/mutex.hpp>
#include "last_error.h"
#include "log_level.h"
#include "singleton.h"
#include "scoped_handle.h"
#include "str_encode.h"



//use char version as possible as you can
//because wchar_t version is based on char version: all wchar_t string will be converted to char string
class CLoggerImpl : public Singleton<CLoggerImpl>
{
    friend class Singleton<CLoggerImpl>;

private:
    CLoggerImpl()
        : m_write_len(0)
    {
    }

public:
    ~CLoggerImpl()
    {
    }

public:
    //if dir is empty, use exe dir
    //when log file increases over max_size, we will create new log file
    //if max_size is 0, use 10 * 1024 * 1024 (10MB)
    bool init(const std::string& dir, const unsigned long max_size);
    bool init(const std::wstring& dir, const unsigned long max_size)
    {
        return init(widestr2ansistr(dir), max_size);
    }

    bool log_string(const LOG_LEVEL level, const char* file, const int line, const std::string& s);
    bool log_string(const LOG_LEVEL level, const char* file, const int line, const std::wstring& ws)
    {
        return log_string(level, file, line, widestr2ansistr(ws));
    }

    bool log_bytes(const LOG_LEVEL level, const char* file, const int line, const void *buf, const unsigned long len, const std::string& prefix);
    bool log_bytes(const LOG_LEVEL level, const char* file, const int line, const void *buf, const unsigned long len, const std::wstring& wprefix)
    {
        return log_bytes(level, file, line, buf, len, widestr2ansistr(wprefix));
    }

    bool log_last_error(const LOG_LEVEL level, const char* file, const int line, CLastError& e, const std::string& prefix);
    bool log_last_error(const LOG_LEVEL level, const char* file, const int line, CLastError& e, const std::wstring& wprefix)
    {
        return log_last_error(level, file, line, e, widestr2ansistr(wprefix));
    }

private:
    static std::string build_prefix(const LOG_LEVEL level, const char* file, const int line);
    static HANDLE new_log_file(const std::string& log_file_dir, const std::string& log_file_name);
    bool write(const LOG_LEVEL level, const std::string& s);

private:
    std::string m_log_file_dir;
    std::string m_log_file_name;
    unsigned long m_log_file_max_size;//in bytes
    boost::mutex m_log_file_lock;
    scoped_handle<true> m_log_file_handle;
    unsigned long m_write_len;
};


