#include <Windows.h>
#include <Psapi.h>
#include <boost/smart_ptr.hpp>
#include "str_encode.h"
#include "logger.h"
#include "process_path_query.h"


#pragma comment(lib, "Psapi.lib")


CProcessPathQuery::fnQueryFullProcessImageNameW CProcessPathQuery::m_s_fnQueryFullProcessImageNameW = NULL;
CProcessPathQuery::fnGetProcessImageFileNameW CProcessPathQuery::m_s_fnGetProcessImageFileNameW = NULL;


CProcessPathQuery::CProcessPathQuery(void)
{
}

CProcessPathQuery::~CProcessPathQuery(void)
{
}

FARPROC CProcessPathQuery::load_function(const tstring& module_name, const tstring& func_name)
{
    HMODULE hModule = GetModuleHandle(module_name.c_str());
    if (NULL == hModule)
    {
        ErrorLogLastErr(CLastError(), TSTR("GetModuleHandle[%s] fail"), module_name.c_str());
        return NULL;
    }
    else
    {
        FARPROC func_addr = GetProcAddress(hModule, tstr2ansistr(func_name).c_str());
        if (NULL == func_addr)
        {
            ErrorLogLastErr(CLastError(), TSTR("GetProcAddress[%s:%s] fail"), module_name.c_str(), func_name.c_str());
        }
        return func_addr;
    }
}

bool CProcessPathQuery::set_privilege(const tstring& privilege_name, const bool enable)
{
    bool bReturn = false;

    HANDLE hToken = NULL;
    do 
    {
        if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
        {
            ErrorLogLastErr(CLastError(), TSTR("OpenProcessToken fail"));
            break;
        }

        LUID luid = {0};
        if (!LookupPrivilegeValue(NULL,         // lookup privilege on local system
            privilege_name.c_str(),                      // privilege to lookup
            &luid))                             // receives LUID of privilege
        {
            ErrorLogLastErr(CLastError(), TSTR("LookupPrivilegeValue[%s] fail"),
                privilege_name.c_str());
            break;
        }

        TOKEN_PRIVILEGES tp = {0};
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        if (enable)
        {
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        }
        else
        {
            tp.Privileges[0].Attributes = 0;
        }

        // Enable the privilege or disable all privileges.
        BOOL adjust_success = AdjustTokenPrivileges(hToken, FALSE, &tp,
            sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);
        CLastError e;
        if (!adjust_success)
        {
            ErrorLogLastErr(e, TSTR("AdjustTokenPrivileges fail when %s [%s]"),
                enable ? TSTR("enable") : TSTR("disable"), privilege_name.c_str());
            break;
        }

        if (e.code() == ERROR_NOT_ALL_ASSIGNED)
        {
            ErrorLog(TSTR("not all privileges were assigned when %s [%s]"),
                enable ? TSTR("enable") : TSTR("disable"), privilege_name.c_str());
            break;
        }

        bReturn = true;

    } while (false);

    if (hToken)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }

    return bReturn;
}

bool CProcessPathQuery::init()
{
    m_s_fnQueryFullProcessImageNameW = reinterpret_cast<fnQueryFullProcessImageNameW>(load_function(TSTR("Kernel32.dll"), TSTR("QueryFullProcessImageNameW")));
    m_s_fnGetProcessImageFileNameW = reinterpret_cast<fnGetProcessImageFileNameW>(load_function(TSTR("Psapi.dll"), TSTR("GetProcessImageFileNameW")));
    if (NULL == m_s_fnGetProcessImageFileNameW)
    {
        m_s_fnGetProcessImageFileNameW = reinterpret_cast<fnGetProcessImageFileNameW>(load_function(TSTR("Kernel32.dll"), TSTR("GetProcessImageFileNameW")));
    }

    return set_privilege(SE_DEBUG_NAME, true);
}

tstring CProcessPathQuery::query(const DWORD pid)
{
    tstring s;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (NULL == hProcess)
    {
        ErrorLogLastErr(CLastError(), TSTR("OpenProcess[%lu] fail"), pid);
    }
    else
    {
        s = query(hProcess);

        CloseHandle(hProcess);
        hProcess = NULL;
    }

    return s;
}

tstring CProcessPathQuery::query(HANDLE hProcess)
{
    std::wstring ws;

    for (unsigned char times_index = 0; times_index != 10; ++times_index)
    {
        const DWORD buf_size = times_index * 500;
        boost::scoped_array<wchar_t> buf(new wchar_t[buf_size]);
        memset(buf.get(), 0, sizeof(wchar_t) * buf_size);

        BOOL query_success = false;
        DWORD query_size = buf_size;

        boost::scoped_ptr<CLastError> e;
        if (m_s_fnQueryFullProcessImageNameW)
        {
            query_success = m_s_fnQueryFullProcessImageNameW(hProcess, 0, buf.get(), &query_size);
            e.reset(new CLastError());
        }
        else if (m_s_fnGetProcessImageFileNameW)
        {
            query_size = m_s_fnGetProcessImageFileNameW(hProcess, buf.get(), query_size);
            e.reset(new CLastError());

            query_success = (query_size != 0);
        }
        else
        {
            query_size = GetModuleFileNameExW(hProcess, NULL, buf.get(), query_size);
            e.reset(new CLastError());

            query_success = (query_size != 0);
        }

        if (query_success)
        {
            ws.append(buf.get(), query_size);
            break;
        }

        if (e->code() != ERROR_INSUFFICIENT_BUFFER)
        {
            ErrorLogLastErr(*(e.get()), TSTR("QueryFullProcessImageNameW fail"));
            break;
        }
    }

    return widestr2tstr(ws);
}

