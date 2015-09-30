#include "logger_impl.h"
#include "logger.h"


bool InitLog(const std::string& dir, const unsigned long max_size)
{
    return CLoggerImpl::get_instance_ref().init(dir, max_size);
}

bool InitLog(const std::wstring& dir, const unsigned long max_size)
{
    return CLoggerImpl::get_instance_ref().init(dir, max_size);
}

bool _Log(const LOG_LEVEL level, const char *file, const int line, const std::string& s)
{
    return CLoggerImpl::get_instance_ref().log_string(level, file, line, s);
}

bool _Log(const LOG_LEVEL level, const char *file, const int line, const std::wstring& ws)
{
    return CLoggerImpl::get_instance_ref().log_string(level, file, line, ws);
}

bool _LogBytes(const LOG_LEVEL level, const char *file, const int line, const void *buf, const unsigned long len, const std::string& prefix)
{
    return CLoggerImpl::get_instance_ref().log_bytes(level, file, line, buf, len, prefix);
}

bool _LogBytes(const LOG_LEVEL level, const char *file, const int line, const void *buf, const unsigned long len, const std::wstring& wprefix)
{
    return CLoggerImpl::get_instance_ref().log_bytes(level, file, line, buf, len, wprefix);
}

bool _LogLastErr(const LOG_LEVEL level, const char *file, const int line, CLastErrorFormat& e, const std::string& prefix)
{
    return CLoggerImpl::get_instance_ref().log_last_error(level, file, line, e, prefix);
}

bool _LogLastErr(const LOG_LEVEL level, const char *file, const int line, CLastErrorFormat& e, const std::wstring& wprefix)
{
    return CLoggerImpl::get_instance_ref().log_last_error(level, file, line, e, wprefix);
}


