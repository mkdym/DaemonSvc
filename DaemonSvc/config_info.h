#pragma once
#include <string>
#include <Windows.h>
#include <boost/algorithm/string.hpp>
#include "tdef.h"
#include "time_point_task.h"
#include "logger.h"


enum CMD_RUN_AS_TYPE
{
    AS_LOCAL,                   //run in local context
    AS_LOGON_USER,              //run as one logon user
    AS_ALL_LOGON_USERS,         //run as all logon users
};


inline CMD_RUN_AS_TYPE cast_run_as_from_string(const std::string& s)
{
    if (boost::algorithm::iequals("all", s))
    {
        return AS_ALL_LOGON_USERS;
    }
    else if (boost::algorithm::iequals("first", s))
    {
        return AS_LOGON_USER;
    }
    else if (boost::algorithm::iequals("local", s))
    {
        return AS_LOCAL;
    }
    else
    {
        ErrorLogA("unknown run_as string: %s, treat as local", s.c_str());
        return AS_LOCAL;
    }
}

inline std::string cast_run_as_to_string(const CMD_RUN_AS_TYPE& run_as)
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
        ErrorLogA("unknown run_as type: %d, treat as local", run_as);
        s = "local";
        break;
    }
    return s;
}


struct TaskInfoBase
{
    CMD_RUN_AS_TYPE run_as;
    bool show_window;
    tstring cmd;

    TaskInfoBase()
        : run_as(AS_LOCAL)
        , show_window(true)
    {
    }

    virtual ~TaskInfoBase()
    {
    }
};


struct TimeIntervalTaskInfo : public TaskInfoBase
{
    DWORD interval_seconds;

    TimeIntervalTaskInfo()
        : interval_seconds(0)
    {
    }
};


struct TimePointTaskInfo : public TaskInfoBase
{
    //todo
};


struct ProcNonExistTaskInfo : public TaskInfoBase
{
    tstring proc_path;
    DWORD interval_seconds;

    ProcNonExistTaskInfo()
        : interval_seconds(0)
    {
    }
};



