#pragma once
#include <Windows.h>
#include "scoped_handle.h"
#include "singleton.h"
#include "tdef.h"


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
    scoped_handle<false> m_mutex;
};


