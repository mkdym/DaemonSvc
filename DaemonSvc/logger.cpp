#include "logger_impl.h"
#include "logger.h"



//I do not consider thread-safety
//calling InitLog in multi-threads is not a reasonable behavior
static LOG_LEVEL g_max_level = LOG_INFO;



bool InitLog(const std::string& dir, const unsigned long max_size, const LOG_LEVEL max_level)
{
    g_max_level = max_level;
    return CLoggerImpl::get_instance_ref().init(dir, max_size);
}

bool InitLog(const std::wstring& dir, const unsigned long max_size, const LOG_LEVEL max_level)
{
    g_max_level = max_level;
    return CLoggerImpl::get_instance_ref().init(dir, max_size);
}

bool _Log(const LOG_LEVEL level, const char *file, const int line, const std::string& s)
{
    if (level > g_max_level)
    {
        return true;
    }
    else
    {
        return CLoggerImpl::get_instance_ref().log_string(level, file, line, s);
    }
}

bool _Log(const LOG_LEVEL level, const char *file, const int line, const std::wstring& ws)
{
    if (level > g_max_level)
    {
        return true;
    }
    else
    {
        return CLoggerImpl::get_instance_ref().log_string(level, file, line, ws);
    }
}

bool _LogBytes(const LOG_LEVEL level, const char *file, const int line,
               const void *buf, const unsigned long len, const std::string& prefix)
{
    if (level > g_max_level)
    {
        return true;
    }
    else
    {
        return CLoggerImpl::get_instance_ref().log_bytes(level, file, line, buf, len, prefix);
    }
}

bool _LogBytes(const LOG_LEVEL level, const char *file, const int line,
               const void *buf, const unsigned long len, const std::wstring& wprefix)
{
    if (level > g_max_level)
    {
        return true;
    }
    else
    {
        return CLoggerImpl::get_instance_ref().log_bytes(level, file, line, buf, len, wprefix);
    }
}

bool _LogLastErr(const LOG_LEVEL level, const char *file, const int line, const std::string& prefix)
{
    if (level > g_max_level)
    {
        return true;
    }
    else
    {
        CLastErrorFormat e;
        return CLoggerImpl::get_instance_ref().log_last_error(level, file, line, e, prefix);
    }
}

bool _LogLastErr(const LOG_LEVEL level, const char *file, const int line, const std::wstring& wprefix)
{
    if (level > g_max_level)
    {
        return true;
    }
    else
    {
        CLastErrorFormat e;
        return CLoggerImpl::get_instance_ref().log_last_error(level, file, line, e, wprefix);
    }
}

bool _LogLastErrEx(const LOG_LEVEL level, const char *file, const int line,
                 CLastErrorFormat& e, const std::string& prefix)
{
    if (level > g_max_level)
    {
        return true;
    }
    else
    {
        return CLoggerImpl::get_instance_ref().log_last_error(level, file, line, e, prefix);
    }
}

bool _LogLastErrEx(const LOG_LEVEL level, const char *file, const int line,
                 CLastErrorFormat& e, const std::wstring& wprefix)
{
    if (level > g_max_level)
    {
        return true;
    }
    else
    {
        return CLoggerImpl::get_instance_ref().log_last_error(level, file, line, e, wprefix);
    }
}




