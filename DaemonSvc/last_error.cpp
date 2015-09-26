#include <boost/algorithm/string.hpp>
#include "last_error.h"


void CLastError::translate()
{
    tchar *buf = NULL;

    const DWORD count = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        m_code,
        0,//MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<tchar *>(&buf),
        0,
        NULL);

    if (0 == count)
    {
        const DWORD e = GetLastError();
        printf_s("FormatMessage fail, error code: %lu\r\n", e);
    }
    else
    {
        m_str.append(buf, count);
        boost::algorithm::trim_right_if(m_str, boost::algorithm::is_any_of(TSTR("\r\n")));
    }

    LocalFree(buf);
}


void print_last_err(const CLastError& e, const tchar* prefix, ...)
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

    tstring s;
    s.append(buf, count);

    _tprintf_s(TSTR("%s, error code: %lu, error msg: %s\r\n"), s.c_str(), e.code(), e.str().c_str());
}


