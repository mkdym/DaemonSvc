#pragma once
#include <vector>
#include <Windows.h>
#include "tdef.h"
#include "process_path_query.h"


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
    //if query_full_path is true, you should call CProcessPathQuery::init() first
    //typically after logger module init at process start
    CProcessScanner(const bool query_full_path);
    ~CProcessScanner(void);

public:
    bool next(ProcessInfo& info);

private:
    bool init();

private:
    HANDLE m_hSnapshot;
    bool m_first_enum;
    bool m_query_full_path;
    bool m_init_success;

    CProcessPathQuery m_process_path_query;
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
// remarks:
//************************************
void find_pids_by_path(const tstring& path,
                       std::vector<DWORD>& pids,
                       const bool only_first = false,
                       const bool exactly_match = true);


