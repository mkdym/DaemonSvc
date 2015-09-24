#pragma once
#include <Windows.h>
#include "tdef.h"


namespace ProcessCreator
{
    //************************************
    // brief:    create process use the same token of source process
    // name:     create_process_as_same_token
    // param:    HANDLE hSourceProcess              source process handle. the process must have the PROCESS_QUERY_INFORMATION access permission
    // param:    const tstring & command            command line to execute
    // param:    DWORD & created_pid                if success, return created process id
    // param:    const DWORD creation_flags         see CreateProcessAsUser's parameter "dwCreationFlags" in MSDN
    // param:    const tstring & work_dir           see CreateProcessAsUser's parameter "lpCurrentDirectory" in MSDN
    // param:    const int show_window_flag         see ShowWindow in MSDN
    // return:   HANDLE                             handle of created process
    // remarks:  implemented by Windows API "CreateProcessAsUser"
    //           @@@@@do not support Windows 2000: CreateProcessAsUser will fail with error code 1314 on Windows 2000, I don't know why
    //************************************
    HANDLE create_process_as_same_token(HANDLE hSourceProcess,
        const tstring& command,
        DWORD& created_pid,
        const DWORD creation_flags = CREATE_NEW_CONSOLE,
        const tstring& work_dir = TSTR(""),
        const int show_window_flag = SW_SHOWNORMAL);

    // see its overload function comment
    // param:    const DWORD pid                    source process id
    HANDLE create_process_as_same_token(const DWORD pid,
        const tstring& command,
        DWORD& created_pid,
        const DWORD creation_flags = CREATE_NEW_CONSOLE,
        const tstring& work_dir = TSTR(""),
        const int show_window_flag = SW_SHOWNORMAL);

    // create process in current context
    // implemented by Windows API "CreateProcess"
    // see create_process_as_same_token
    HANDLE create_process_in_local_context(const tstring& command,
        DWORD& created_pid,
        const DWORD creation_flags = CREATE_NEW_CONSOLE,
        const tstring& work_dir = TSTR(""),
        const int show_window_flag = SW_SHOWNORMAL);
}

