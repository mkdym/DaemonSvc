#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "last_error_recover.h"


//if you will modify the source code, be aware that you should recover last error code
//because someone may use like this:
//    reset(CreateFile(...));
//    const DWORD e = GetLastError();
//if you don't recover last error code, then e may be CloseHandle's last error code

//if is_file_handle is true, invalid handle value will be INVALID_HANDLE_VALUE
//otherwise is NULL
template<bool is_file_handle = false>
class scoped_handle : public boost::noncopyable
{
public:
    scoped_handle()
        : h_invalid_(is_file_handle ? INVALID_HANDLE_VALUE : NULL)
    {
        //do not assign h_ in initialization list
        //in order to avoid errors because of initialization sequence
        h_ = h_invalid_;
    }

    //HANDLE is a ptr, so do not need HANDLE* or HANDLE&
    //just assign by value
    scoped_handle(HANDLE h)
        : h_(h)
    {
    }

    ~scoped_handle()
    {
        destory();
    }

    //you should ensure not self-assignment
    void reset(HANDLE h)
    {
        destory();
        h_ = h;
    }

    bool valid() const
    {
        return h_invalid_ != h_;
    }

    HANDLE get() const
    {
        return h_;
    }

private:
    void destory()
    {
        if (h_invalid_ != h_)
        {
            //CloseHandle will set last error code
            //so we should recover it
            //someone may use reset(CreateFile(...))
            last_error_recover r;

            CloseHandle(h_);
            h_ = h_invalid_;
        }
    }

private:
    HANDLE h_;
    const HANDLE h_invalid_;
};



