#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>


class last_error_recover : public boost::noncopyable
{
public:
    last_error_recover()
        : code_(GetLastError())
    {
    }

    last_error_recover(const DWORD code)
        : code_(code)
    {
    }

    ~last_error_recover()
    {
        SetLastError(code_);
    }

private:
    const DWORD code_;
};



