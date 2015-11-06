#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "last_error_recover.h"


//if you will modify the source code, be aware that you should recover last error code
//because someone may use like this:
//    reset(CreateFile(...));
//    const DWORD e = GetLastError();
//if you don't recover last error code, then e may be CloseHandle's last error code
//invalid_value may be NULL or INVALID_HANDLE_VALUE
//todo: make type 'HANDLE' and func 'CloseHandle' be template parameters
template<HANDLE invalid_value = NULL>
class scoped_handle : public boost::noncopyable
{
public:
    scoped_handle()
        : h_(invalid_value)
    {
    }

    scoped_handle(const HANDLE &h)
        : h_(h)
    {
    }

    ~scoped_handle()
    {
        destory();
    }

    //you should ensure not self-assignment
    void reset(const HANDLE &h)
    {
        destory();
        h_ = h;
    }

    void destory()
    {
        if (h_ != invalid_value)
        {
            //CloseHandle will set last error code
            //so we should recover it
            //someone may use reset(CreateFile(...))
            last_error_recover r;

            CloseHandle(h_);
            h_ = invalid_value;
        }
    }

    bool valid() const
    {
        return h_ != invalid_value;
    }

    HANDLE& get_ref()
    {
        return h_;
    }

    HANDLE* get_ptr()
    {
        return &h_;
    }

private:
    HANDLE h_;
};



