#include <Windows.h>
#include <Tlhelp32.h>
#include "boost_algorithm_string.h"
#include "logger.h"
#include "os_ver.h"
#include "str_encode.h"
#include "process_scanner.h"


CProcessScanner::CProcessScanner(const bool query_full_path)
    : m_first_enum(true)
    , m_query_full_path(query_full_path)
{
    m_hSnapshot.reset(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (!m_hSnapshot.valid())
    {
        ErrorLogLastErr("CreateToolhelp32Snapshot fail");
    }
}

CProcessScanner::~CProcessScanner(void)
{
}

bool CProcessScanner::next(ProcessInfo& info)
{
    if (!m_hSnapshot.valid())
    {
        ErrorLog("process scanner init fail");
        return false;
    }

    bool bReturn = false;

    do 
    {
        PROCESSENTRY32 pe32 = {0};
        pe32.dwSize = sizeof(pe32);
        if (m_first_enum)
        {
            if (!Process32First(m_hSnapshot.get(), &pe32))
            {
                ErrorLogLastErr("Process32First fail");
                break;
            }
            m_first_enum = false;
        }
        else
        {
            if (!Process32Next(m_hSnapshot.get(), &pe32))
            {
                CLastErrorFormat e;
                if (e.code() != ERROR_NO_MORE_FILES)
                {
                    ErrorLogLastErrEx(e, "Process32Next fail");
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
            static OS_VER os_v = get_os_version();

            //skip "System Process" and "System"
            //on Windows 2000, "System Process" id is 0, "System" is 4
            //on other Windows, "System Process" id is 0, "System" is 8
            //todo: maybe not correct
            if (0 == info.pid || 4 == info.pid || 8 == info.pid)
            {
                //system process
            }
            else if (os_v.v >= OS_VER::WIN_VISTA
                && boost::algorithm::iequals(info.exe_name, TSTR("audiodg.exe")))
            {
                //can not get process "audiodg.exe" full path
            }
            else
            {
                bool native_name = false;
                info.full_path = m_process_path_query.query(info.pid, native_name);
                if (!info.full_path.empty() && native_name)
                {
                    //convert native name to dos name
                    info.full_path = widestr2tstr(m_dos_path_converter.to_dos_path(tstr2widestr(info.full_path)));
                }
                //convert short pathname
                if (!info.full_path.empty())
                {
                    info.full_path = CDosPathConverter::to_long_path_name(info.full_path);
                }
                if (info.full_path.empty())
                {
                    ErrorLog(TSTR("can not get process [%s:%d] full path"), info.exe_name.c_str(), info.pid);
                }
            }
        }

        bReturn = true;

    } while (false);

    return bReturn;
}


void find_pids_by_path(const tstring& path,
                       std::vector<DWORD>& pids,
                       const bool only_first /*= false*/,
                       const bool exactly_match /*= true*/)
{
    bool need_query_full_path = false;
    if (tstring::npos != path.find_first_of(TSTR("\\/")))
    {
        need_query_full_path = true;
    }

    ProcessInfo pi;
    CProcessScanner ps(need_query_full_path);
    while (ps.next(pi))
    {
        if (need_query_full_path)
        {
            if (exactly_match)
            {
                if (boost::algorithm::iequals(pi.full_path, path))
                {
                    pids.push_back(pi.pid);
                    if (only_first)
                    {
                        break;
                    }
                }
            }
            else
            {
                if (boost::algorithm::iends_with(pi.full_path, path))
                {
                    pids.push_back(pi.pid);
                    if (only_first)
                    {
                        break;
                    }
                }
            }
        }
        else
        {
            if (boost::algorithm::iequals(pi.exe_name, path))
            {
                pids.push_back(pi.pid);
                if (only_first)
                {
                    break;
                }
            }
        }
    }
}

