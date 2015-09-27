#include <Windows.h>
#include <Psapi.h>
#include <boost/smart_ptr.hpp>
#include "str_encode.h"
#include "logger.h"
#include "windows_util.h"
#include "process_path_query.h"


#pragma comment(lib, "Psapi.lib")



CProcessPathQuery::CProcessPathQuery(void)
{
    //todo: 此处是否恰当
    //如果使用非静态变量，每次构造都load，记录日志，则会有太多错误日志；不记，有可能给排查问题带来不便
    //如果使用静态变量，只load一次，那么就需要考虑线程同步的问题，加锁或者原子操作
    //最后，还是考虑使用非静态变量，不记日志
    m_fnQueryFullProcessImageNameW = reinterpret_cast<fnQueryFullProcessImageNameW>
        (WindowsUtil::load_function(TSTR("Kernel32.dll"), TSTR("QueryFullProcessImageNameW"), false));

    m_fnGetProcessImageFileNameW = reinterpret_cast<fnGetProcessImageFileNameW>
        (WindowsUtil::load_function(TSTR("Psapi.dll"), TSTR("GetProcessImageFileNameW"), false));

    if (NULL == m_fnGetProcessImageFileNameW)
    {
        m_fnGetProcessImageFileNameW = reinterpret_cast<fnGetProcessImageFileNameW>
            (WindowsUtil::load_function(TSTR("Kernel32.dll"), TSTR("GetProcessImageFileNameW"), false));
    }
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
        if (m_fnQueryFullProcessImageNameW)
        {
            query_success = m_fnQueryFullProcessImageNameW(hProcess, 0, buf.get(), &query_size);
            e.reset(new CLastError());
            func_name = TSTR("QueryFullProcessImageNameW");
        }
        else if (m_fnGetProcessImageFileNameW)
        {
            query_size = m_fnGetProcessImageFileNameW(hProcess, buf.get(), query_size);
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

