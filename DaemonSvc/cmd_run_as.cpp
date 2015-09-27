#include <vector>
#include <Windows.h>
#include "boost_algorithm_string.h"
#include "process_creator.h"
#include "process_scanner.h"
#include "logger.h"
#include "cmd_run_as.h"


RUN_AS_TYPE cast_run_as_from_string(const std::string& s)
{
    std::string s_lower = boost::algorithm::to_lower_copy(s);
    boost::algorithm::trim(s_lower);

    if (s_lower == "all")
    {
        return AS_ALL_LOGON_USERS;
    }
    else if (s_lower == "first")
    {
        return AS_LOGON_USER;
    }
    else if (s_lower == "local")
    {
        return AS_LOCAL;
    }
    else
    {
        ErrorLogA("can not cast string[%s] to RUN_AS_TYPE", s_lower.c_str());
        return AS_UNKNOWN;
    }
}

std::string cast_run_as_to_string(const RUN_AS_TYPE& run_as)
{
    std::string s;
    switch (run_as)
    {
    case AS_ALL_LOGON_USERS:
        s = "all";
        break;

    case AS_LOGON_USER:
        s = "first";
        break;

    case AS_LOCAL:
        s = "local";
        break;

    default:
        ErrorLogA("unknown RUN_AS_TYPE: %d", run_as);
        s = "unknown";
        break;
    }
    return s;
}


bool cmd_run_as(const tstring& command,
                const RUN_AS_TYPE& run_as,
                const bool show_window /*= true*/)
{
    InfoLogA("begin exec");

    bool execute_success = false;
    const unsigned short sw_flag = show_window ? SW_SHOWNORMAL : SW_HIDE;

    std::vector<HANDLE> processes;

    switch (run_as)
    {
    case AS_LOCAL:
        {
            DWORD created_pid = 0;
            HANDLE hProcess = ProcessCreator::create_process_in_local_context(command,
                created_pid, CREATE_NEW_CONSOLE, TSTR(""), sw_flag);
            if (hProcess)
            {
                InfoLogA("create_process_in_local_context success, pid=%lu", created_pid);
                processes.push_back(hProcess);
            }
            else
            {
                ErrorLogA("create_process_in_local_context fail");
            }
        }
        break;

    case AS_LOGON_USER:
    case AS_ALL_LOGON_USERS:
        {
            std::vector<DWORD> pids;
            find_pids_by_path(TSTR("explorer.exe"), pids);
            for (std::vector<DWORD>::const_iterator iter_pid = pids.begin();
                iter_pid != pids.end();
                ++iter_pid)
            {
                InfoLogA("explorer.exe pid=%lu", *iter_pid);
                DWORD created_pid = 0;
                HANDLE hProcess = ProcessCreator::create_process_as_same_token(*iter_pid,
                    command, created_pid, CREATE_NEW_CONSOLE, TSTR(""), sw_flag);
                if (hProcess)
                {
                    InfoLogA("create_process_as_same_token success, pid=%lu", created_pid);
                    processes.push_back(hProcess);
                    if (run_as == AS_LOGON_USER)
                    {
                        break;
                    }
                }
                else
                {
                    ErrorLog(TSTR("create_process_as_same_token fail, pid=%lu, cmd=[%s]"),
                        *iter_pid, command.c_str());
                }
            }

            if (processes.empty())
            {
                ErrorLogA("no new process in user context was created, all fail, try create in local context");
                DWORD created_pid = 0;
                HANDLE hProcess = ProcessCreator::create_process_in_local_context(command,
                    created_pid, CREATE_NEW_CONSOLE, TSTR(""), sw_flag);
                if (hProcess)
                {
                    InfoLogA("create_process_in_local_context success, pid=%lu", created_pid);
                    processes.push_back(hProcess);
                }
                else
                {
                    ErrorLogA("create_process_in_local_context fail");
                }
            }
        }
        break;

    default:
        ErrorLogA("unknown run_as type: %d", run_as);
        break;
    }

    if (!processes.empty())
    {
        //最多伺候3秒，不必看结果，让守护进程去看
        WaitForMultipleObjects(processes.size(), &processes[0], TRUE, 3 * 1000);
        for (std::vector<HANDLE>::iterator iter_process = processes.begin();
            iter_process != processes.end();
            ++iter_process)
        {
            CloseHandle(*iter_process);
            *iter_process = NULL;
        }

        execute_success = true;
    }
    else
    {
        execute_success = false;
    }

    InfoLogA("end exec with %s", execute_success ? "success" : "fail");
    return execute_success;
}


