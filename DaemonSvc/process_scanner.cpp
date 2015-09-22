#include <Windows.h>
#include <Tlhelp32.h>
#include "logger.h"
#include "process_scanner.h"


CProcessScanner::CProcessScanner(const bool query_full_path)
    : m_hSnapshot(INVALID_HANDLE_VALUE)
    , m_first_enum(true)
    , m_query_full_path(query_full_path)
{
    m_init_success = init();
}

CProcessScanner::~CProcessScanner(void)
{
    if (m_hSnapshot != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hSnapshot);
        m_hSnapshot = INVALID_HANDLE_VALUE;
    }
}

bool CProcessScanner::next(ProcessInfo& info)
{
    if (!m_init_success)
    {
        ErrorLogA("process scanner init fail");
        return false;
    }

    bool bReturn = false;

    do 
    {
        PROCESSENTRY32 pe32 = {0};
        pe32.dwSize = sizeof(pe32);
        if (m_first_enum)
        {
            if (!Process32First(m_hSnapshot, &pe32))
            {
                ErrorLogLastErr(CLastError(), TSTR("Process32First fail"));
                break;
            }
            m_first_enum = false;
        }
        else
        {
            if (!Process32Next(m_hSnapshot, &pe32))
            {
                CLastError e;
                if (e.code() != ERROR_NO_MORE_FILES)
                {
                    ErrorLogLastErr(e, TSTR("Process32Next fail"));
                }
                break;
            }
        }

        info.pid = pe32.th32ProcessID;
        info.ppid = pe32.th32ParentProcessID;
        info.thread_count = pe32.cntThreads;
        info.exe_name = pe32.szExeFile;

        info.full_path.clear();
        if (m_query_full_path)
        {
            if (0 == info.pid || 4 == info.pid)
            {
                //system process
            }
            else
            {
                info.full_path = m_process_path_query.query(info.pid);
            }
        }

        bReturn = true;

    } while (false);

    return bReturn;
}

bool CProcessScanner::init()
{
    bool bReturn = false;

    do 
    {
        m_hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (INVALID_HANDLE_VALUE == m_hSnapshot)
        {
            ErrorLogLastErr(CLastError(), TSTR("CreateToolhelp32Snapshot fail"));
            break;
        }

        bReturn = true;

    } while (false);

    return bReturn;
}
