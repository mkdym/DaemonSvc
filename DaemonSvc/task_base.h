#pragma once
#include <boost/noncopyable.hpp>


class CTaskBase : public boost::noncopyable
{
public:
    CTaskBase()
    {
    }

    virtual ~CTaskBase()
    {
    }

public:
    virtual bool is_started() const = 0;
    virtual bool start() = 0;
    virtual void stop() = 0;
};


