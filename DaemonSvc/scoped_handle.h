#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>


//if is_file_handle is true, invalid handle value will be INVALID_HANDLE_VALUE
//otherwise is NULL
template<bool is_file_handle = false>
class scoped_handle : public boost::noncopyable
{
public:
    scoped_handle()
        : h_(is_file_handle ? INVALID_HANDLE_VALUE : NULL)
    {
    }

    //HANDLE is a ptr, so do not need HANDLE* or HANDLE&
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
        bool ret = false;

        if (is_file_handle)
        {
            if (h_ != INVALID_HANDLE_VALUE)
            {
                ret = true;
            }
        }
        else
        {
            if (h_)
            {
                ret = true;
            }
        }

        return ret;
    }

    HANDLE get() const
    {
        return h_;
    }

private:
    void destory()
    {
        if (is_file_handle)
        {
            if (h_ != INVALID_HANDLE_VALUE)
            {
                CloseHandle(h_);
                h_ = INVALID_HANDLE_VALUE;
            }
        }
        else
        {
            if (h_)
            {
                CloseHandle(h_);
                h_ = NULL;
            }
        }
    }

private:
    HANDLE h_;
};



