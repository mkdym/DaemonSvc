#include <boost/smart_ptr.hpp>
#include "scoped_handle.h"
#include "logger.h"
#include "process_creator.h"



HANDLE ProcessCreator::create_process_as_same_token(HANDLE hSourceProcess,
                                    const tstring& command,
                                    DWORD& created_pid,
                                    const DWORD creation_flags/* = CREATE_NEW_CONSOLE*/,
                                    const tstring& work_dir/* = TSTR("")*/,
                                    const unsigned short show_window_flag/* = SW_SHOWNORMAL*/)
{
    HANDLE hTargetProcess = NULL;

    do 
    {
        scoped_handle<> hSourceToken;
        //if (!OpenProcessToken(hSourceProcess, TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY, hSourceToken.get_ptr()))
        if (!OpenProcessToken(hSourceProcess, TOKEN_DUPLICATE, hSourceToken.get_ptr()))
        {
            ErrorLogLastErr("OpenProcessToken fail");
            break;
        }

        scoped_handle<> hTargetToken;
        if (!DuplicateTokenEx(hSourceToken.get_ref(),
            MAXIMUM_ALLOWED,
            NULL,
            SecurityDelegation,
            TokenPrimary,
            hTargetToken.get_ptr()))
        {
            ErrorLogLastErr("DuplicateTokenEx fail");
            break;
        }

        STARTUPINFO si = {0};
        si.cb = sizeof(si);
        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = show_window_flag;

        PROCESS_INFORMATION pi = {0};

        boost::scoped_array<tchar> cmd_array(new tchar[command.size() + 1]);
        memset(cmd_array.get(), 0, (command.size() + 1) * sizeof(tchar));
        memcpy_s(cmd_array.get(), command.size() * sizeof(tchar),
            command.c_str(), command.size() * sizeof(tchar));

        if (!CreateProcessAsUser(hTargetToken.get_ref(),
            NULL,
            cmd_array.get(),
            NULL,
            NULL,
            FALSE,
            creation_flags,
            NULL,
            work_dir.empty() ? NULL : work_dir.c_str(),
            &si,
            &pi))
        {
            ErrorLogLastErr(TSTR("CreateProcessAsUser fail, cmd=[%s]"), cmd_array.get());
            break;
        }

        CloseHandle(pi.hThread);
        hTargetProcess = pi.hProcess;
        created_pid = pi.dwProcessId;

    } while (false);

    return hTargetProcess;
}

//see its overload function comment
HANDLE ProcessCreator::create_process_as_same_token(const DWORD pid,
                                    const tstring& command,
                                    DWORD& created_pid,
                                    const DWORD creation_flags/* = CREATE_NEW_CONSOLE*/,
                                    const tstring& work_dir/* = TSTR("")*/,
                                    const unsigned short show_window_flag/* = SW_SHOWNORMAL*/)
{
    HANDLE hTargetProcess = NULL;

    HANDLE hSourceProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (NULL == hSourceProcess)
    {
        ErrorLogLastErr("OpenProcess[%lu] fail", pid);
    }
    else
    {
        hTargetProcess = create_process_as_same_token(hSourceProcess,
            command,
            created_pid,
            creation_flags,
            work_dir,
            show_window_flag);

        CloseHandle(hSourceProcess);
        hSourceProcess = NULL;
    }
    return hTargetProcess;
}

//see create_process_as_same_token
HANDLE ProcessCreator::create_process_in_local_context(const tstring& command,
                                       DWORD& created_pid,
                                       const DWORD creation_flags/* = CREATE_NEW_CONSOLE*/,
                                       const tstring& work_dir/* = TSTR("")*/,
                                       const unsigned short show_window_flag/* = SW_SHOWNORMAL*/)
{
    HANDLE hTargetProcess = NULL;

    do 
    {
        STARTUPINFO si = {0};
        si.cb = sizeof(si);
        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = show_window_flag;

        PROCESS_INFORMATION pi = {0};

        boost::scoped_array<tchar> cmd_array(new tchar[command.size() + 1]);
        memset(cmd_array.get(), 0, (command.size() + 1) * sizeof(tchar));
        memcpy_s(cmd_array.get(), command.size() * sizeof(tchar),
            command.c_str(), command.size() * sizeof(tchar));

        if (!CreateProcess(NULL,
            cmd_array.get(),
            NULL,
            NULL,
            FALSE,
            creation_flags,
            NULL,
            work_dir.empty() ? NULL : work_dir.c_str(),
            &si,
            &pi))
        {
            ErrorLogLastErr(TSTR("CreateProcess fail, cmd=[%s]"), cmd_array.get());
            break;
        }

        CloseHandle(pi.hThread);
        hTargetProcess = pi.hProcess;
        created_pid = pi.dwProcessId;

    } while (false);

    return hTargetProcess;
}

