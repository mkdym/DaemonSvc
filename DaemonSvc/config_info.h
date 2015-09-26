#pragma once
#include <string>
#include <Windows.h>
#include "tdef.h"
#include "str_encode.h"
#include "any_lexical_cast.h"
#include "period_time.h"
#include "cmd_run_as.h"



struct TaskInfoBase
{
    RUN_AS_TYPE run_as;
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

    virtual std::string str()
    {
        std::string s = "run_as[" + cast_run_as_to_string(run_as)
            + "], show_window[" + string_lexical_cast<char>(show_window)
            + "], cmd[" + tstr2ansistr(cmd)
            + "]";
        return s;
    }
};


struct TimeIntervalTaskInfo : public TaskInfoBase
{
    DWORD interval_seconds;

    TimeIntervalTaskInfo()
        : interval_seconds(0)
    {
    }

    std::string str()
    {
        std::string s;
        s = "interval_seconds[" + string_lexical_cast<char>(interval_seconds)
            + "], " + TaskInfoBase::str();
        return s;
    }
};


struct TimePointTaskInfo : public TaskInfoBase
{
    PeriodTime pt;

    std::string str()
    {
        std::string s;
        s = "pt[" + pt.str()
            + "], " + TaskInfoBase::str();
        return s;
    }
};


struct ProcNonExistTaskInfo : public TaskInfoBase
{
    tstring proc_path;
    DWORD interval_seconds;

    ProcNonExistTaskInfo()
        : interval_seconds(0)
    {
    }

    std::string str()
    {
        std::string s;
        s = "proc_path[" + tstr2ansistr(proc_path)
            + "], interval_seconds[" + string_lexical_cast<char>(interval_seconds)
            + "], " + TaskInfoBase::str();
        return s;
    }
};



