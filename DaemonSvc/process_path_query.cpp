#include <Windows.h>
#include <Psapi.h>
#include <boost/smart_ptr.hpp>
#include <boost/thread/once.hpp>
#include "str_encode.h"
#include "logger.h"
#include "windows_util.h"
#include "process_path_query.h"


#pragma comment(lib, "Psapi.lib")



static boost::once_flag once_;


typedef BOOL (WINAPI *fnQueryFullProcessImageNameW)(HANDLE hProcess, DWORD dwFlags, LPTSTR lpExeName, PDWORD lpdwSize);
typedef DWORD (WINAPI *fnGetProcessImageFileNameW)(HANDLE hProcess, LPTSTR lpImageFileName, DWORD nSize);

static fnQueryFullProcessImageNameW g_fnQueryFullProcessImageNameW;
static fnGetProcessImageFileNameW g_fnGetProcessImageFileNameW;


static void load_query_funcs()
{
    g_fnQueryFullProcessImageNameW = reinterpret_cast<fnQueryFullProcessImageNameW>
        (WindowsUtil::load_function(TSTR("Kernel32.dll"), TSTR("QueryFullProcessImageNameW")));

    g_fnGetProcessImageFileNameW = reinterpret_cast<fnGetProcessImageFileNameW>
        (WindowsUtil::load_function(TSTR("Psapi.dll"), TSTR("GetProcessImageFileNameW")));

    if (NULL == g_fnGetProcessImageFileNameW)
    {
        g_fnGetProcessImageFileNameW = reinterpret_cast<fnGetProcessImageFileNameW>
            (WindowsUtil::load_function(TSTR("Kernel32.dll"), TSTR("GetProcessImageFileNameW")));
    }
}



CProcessPathQuery::CProcessPathQuery(void)
{
    boost::call_once(once_, load_query_funcs);
}

CProcessPathQuery::~CProcessPathQuery(void)
{
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

        tstring func_name;
        boost::scoped_ptr<CLastError> e;
        if (g_fnQueryFullProcessImageNameW)
        {
            query_success = g_fnQueryFullProcessImageNameW(hProcess, 0, buf.get(), &query_size);
            e.reset(new CLastError());
            func_name = TSTR("QueryFullProcessImageNameW");
        }
        else if (g_fnGetProcessImageFileNameW)
        {
            query_size = g_fnGetProcessImageFileNameW(hProcess, buf.get(), query_size);
            e.reset(new CLastError());
            func_name = TSTR("GetProcessImageFileNameW");

            query_success = (query_size != 0);
        }
        else
        {
            query_size = GetModuleFileNameExW(hProcess, NULL, buf.get(), query_size);
            e.reset(new CLastError());
            func_name = TSTR("GetModuleFileNameExW");

            query_success = (query_size != 0);
        }

        if (query_success)
        {
            ws.append(buf.get(), query_size);
            break;
        }

        if (e->code() != ERROR_INSUFFICIENT_BUFFER)
        {
            ErrorLogLastErr(*(e.get()), TSTR("%s fail"), func_name.c_str());
            break;
        }
    }

    return widestr2tstr(ws);
}

