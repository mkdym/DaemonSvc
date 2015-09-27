#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "tdef.h"


//you should enable debug privilege if you want query other user's process
//typically, you can call WindowsUtil::set_privilege(SE_DEBUG_NAME, true)
class CProcessPathQuery : public boost::noncopyable
{
public:
    CProcessPathQuery(void);
    ~CProcessPathQuery(void);

public:
    tstring query(const DWORD pid);
    tstring query(HANDLE hProcess);

private:
    typedef BOOL (WINAPI *fnQueryFullProcessImageNameW)(HANDLE hProcess, DWORD dwFlags, LPTSTR lpExeName, PDWORD lpdwSize);
    typedef DWORD (WINAPI *fnGetProcessImageFileNameW)(HANDLE hProcess, LPTSTR lpImageFileName, DWORD nSize);

    fnQueryFullProcessImageNameW m_fnQueryFullProcessImageNameW;
    fnGetProcessImageFileNameW m_fnGetProcessImageFileNameW;
};
