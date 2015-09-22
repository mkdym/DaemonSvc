#pragma once
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


