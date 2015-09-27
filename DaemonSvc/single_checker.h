#pragma once
#include <Windows.h>
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
    HANDLE m_mutex;
};


