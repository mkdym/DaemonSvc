#pragma once
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>



typedef boost::function<void()> TaskFunc;



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


