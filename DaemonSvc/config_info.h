#pragma once
#include <string>
#include <Windows.h>
#include <boost/algorithm/string.hpp>
#include "tdef.h"
#include "period_time.h"
#include "cmd_run_as.h"
#include "logger.h"



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



