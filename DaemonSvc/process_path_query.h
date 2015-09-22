#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "tdef.h"


//you should call CProcessPathQuery::init() first
//typically after logger module init at process start
class CProcessPathQuery : public boost::noncopyable
{
public:
    CProcessPathQuery(void);
    ~CProcessPathQuery(void);

public:
    static FARPROC load_function(const tstring& module_name, const tstring& func_name);
    static bool set_privilege(const tstring& privilege_name, const bool enable);

    //not thread-safe
    static bool init();

    tstring query(const DWORD pid);
    tstring query(HANDLE hProcess);

private:
    typedef BOOL (WINAPI *fnQueryFullProcessImageNameW)(HANDLE hProcess, DWORD dwFlags, LPTSTR lpExeName, PDWORD lpdwSize);
    typedef DWORD (WINAPI *fnGetProcessImageFileNameW)(HANDLE hProcess, LPTSTR lpImageFileName, DWORD nSize);

    static fnQueryFullProcessImageNameW m_s_fnQueryFullProcessImageNameW;
    static fnGetProcessImageFileNameW m_s_fnGetProcessImageFileNameW;
};
