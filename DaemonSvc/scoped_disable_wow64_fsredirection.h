#pragma once
#include <boost/noncopyable.hpp>



class scoped_disable_wow64_fsredirection : public boost::noncopyable
{
public:
    scoped_disable_wow64_fsredirection();

    ~scoped_disable_wow64_fsredirection();

private:
    static bool disable(void **ppOldValue);
    static bool revert(void *pOldValue);

private:
    void *_pOldValue;
};


