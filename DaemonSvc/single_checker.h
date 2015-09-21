#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "tdef.h"


class CSingleChecker : public boost::noncopyable
{
private:
    CSingleChecker(void);
    ~CSingleChecker(void);

public:
    static CSingleChecker& GetInstanceRef()
    {
        static CSingleChecker instance;
        return instance;
    }

public:
    bool single(const tstring& mutex_name);

private:
    HANDLE m_mutex;
};


