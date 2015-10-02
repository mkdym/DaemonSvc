#include <Windows.h>
#include <Psapi.h>
#include <boost/smart_ptr.hpp>
#include <boost/thread/once.hpp>
#include "scoped_handle.h"
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

tstring CProcessPathQuery::query(const DWORD pid, bool& native_name)
{
    tstring s;
    native_name = false;

    scoped_handle<false> hProcess(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid));
    if (!hProcess.valid())
    {
        ErrorLogLastErr("OpenProcess[%lu] fail", pid);
    }
    else
    {
        s = query(hProcess.get(), native_name);
    }

    return s;
}

tstring CProcessPathQuery::query(HANDLE hProcess, bool& native_name)
{
    std::wstring ws;
    native_name = false;

    const unsigned char MAX_TIMES = 10;
    const DWORD PER_INCREASE = 500;

    unsigned char times_index = 1;
    for (; times_index != (MAX_TIMES + 1); ++times_index)
    {
        const DWORD buf_size = times_index * PER_INCREASE;

        //do not need memset 0, because will return count
        boost::scoped_array<wchar_t> buf(new wchar_t[buf_size]);

        //Windows Vista/2008 and later versions have function "QueryFullProcessImageNameW"
        if (g_fnQueryFullProcessImageNameW)
        {
            DWORD query_size = buf_size;
            if (g_fnQueryFullProcessImageNameW(hProcess, 0, buf.get(), &query_size))
            {
                //success
                ws.append(buf.get(), query_size);
                break;
            }

            CLastErrorFormat e;
            if (e.code() != ERROR_INSUFFICIENT_BUFFER)
            {
                //fail
                ErrorLogLastErrEx(e, "QueryFullProcessImageNameW fail");
                break;
            }
        }
        else
        {
            //all versions of Windows have function "GetModuleFileNameExW"
            //but fail when process is starting and pe header is not well prepared
            //or 32bits process queries 64bits process(maybe)
            const DWORD query_size_1 = GetModuleFileNameExW(hProcess, NULL, buf.get(), buf_size);
            if (query_size_1)
            {
                //success
                ws.append(buf.get(), query_size_1);
                break;
            }

            CLastErrorFormat e1;
            if (e1.code() != ERROR_INSUFFICIENT_BUFFER)
            {
                //fail
                if (NULL == g_fnGetProcessImageFileNameW)
                {
                    ErrorLogLastErrEx(e1, "GetModuleFileNameExW fail");
                    break;
                }

                //Windows XP and later versions have function "GetProcessImageFileNameW"
                //but it returns a native path
                //try GetProcessImageFileNameW
                const DWORD query_size_2 = g_fnGetProcessImageFileNameW(hProcess, buf.get(), buf_size);
                if (query_size_2)
                {
                    //success
                    native_name = true;//in native name format, see GetProcessImageFileNameW in MSDN
                    ws.append(buf.get(), query_size_2);
                    break;
                }

                CLastErrorFormat e2;
                if (e2.code() != ERROR_INSUFFICIENT_BUFFER)
                {
                    //fail
                    ErrorLogLastErrEx(e1, "GetModuleFileNameExW fail");
                    ErrorLogLastErrEx(e2, "GetProcessImageFileNameW fail");
                    break;
                }
            }
        }
    }

    if (MAX_TIMES == times_index)
    {
        ErrorLog("process path is too long, support max size[%lu]", MAX_TIMES * PER_INCREASE);
    }

    return widestr2tstr(ws);
}

