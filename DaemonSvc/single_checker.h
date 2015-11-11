#pragma once
#include <Windows.h>
#include "scoped_handle.h"
#include "singleton.h"
#include "tdef.h"


//CSingleChecker::single will create a mutex, and hold it for mutual exclusion
//so "mutual exclusion" will effect until you call CSingleChecker::single
class CSingleChecker : public Singleton<CSingleChecker>
{
    friend class Singleton<CSingleChecker>;

private:
    CSingleChecker(void);

public:
    ~CSingleChecker(void);

public:
    bool single(const tstring& mutex_name);

private:
    scoped_handle<> m_mutex;
};


