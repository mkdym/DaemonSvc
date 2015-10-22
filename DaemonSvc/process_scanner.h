#pragma once
#include <vector>
#include <Windows.h>
#include "scoped_handle.h"
#include "tdef.h"
#include "process_path_query.h"
#include "dos_path_converter.h"


struct ProcessInfo
{
    DWORD pid;
    DWORD ppid;
    DWORD thread_count;
    tstring exe_name;
    tstring full_path;

    ProcessInfo()
        : pid(0)
        , ppid(0)
        , thread_count(0)
    {
    }
};



class CProcessScanner
{
public:
    //if query_full_path is true, returned "info" will contains a not empty full path string if no error
    //I skipped 3 pid full path query: 0, 4, 8
    //so you will always get an empty string for the 3 pid
    //because I can not get their path
    //in order to reduce error log, I skipped them
    //      on Windows 2000, "System Process" id is 0, "System" is 4
    //      on other Windows, "System Process" id is 0, "System" is 8
    CProcessScanner(const bool query_full_path);
    ~CProcessScanner(void);

public:
    bool next(ProcessInfo& info);

private:
    scoped_handle<INVALID_HANDLE_VALUE> m_hSnapshot;
    bool m_first_enum;
    bool m_query_full_path;

    CProcessPathQuery m_process_path_query;
    CDosPathConverter m_dos_path_converter;
};



//************************************
// brief:    find process id on the system which matched the process path
// name:     find_pids_by_path
// param:    const tstring & path               process name or relative/full path
// param:    std::vector<DWORD> & pids          found pid list
// param:    const bool only_first              only return the first matched process if true, otherwise return all
//                                              will scan all processes on the system when false
// param:    const bool exactly_match           if exactly_match is false, use iends_with to match path, otherwise, use iequals to match path
//                                              when path does not contain '\' or '/', ignore exactly_match, always treated with true
// return:   void
// remarks:  if path contains '\' or '/', you should call CProcessPathQuery::init() first
//           typically after logger module init at process start
//************************************
void find_pids_by_path(const tstring& path,
                              std::vector<DWORD>& pids,
                              const bool only_first = false,
                              const bool exactly_match = true);




