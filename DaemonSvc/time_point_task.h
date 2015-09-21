#pragma once
#include <Windows.h>
#include <boost/thread.hpp>
#include "task_common.h"
#include "task_base.h"


struct PeriodTime
{
    enum PERIOD_TYPE
    {
        DAILY,
        WEEKLY,
        MONTHLY,
    };

    PERIOD_TYPE type;
    short dayofmonth;
    short dayofweek;
    short hour;
    short minute;
    short second;

    PeriodTime()
        : dayofmonth(0)
        , dayofweek(0)
        , hour(0)
        , minute(0)
        , second(0)
    {
    }
};


//时间点任务：在每天、每周、每月的指定时间执行
class CTimePointTask : public CTaskBase
{
public:
    CTimePointTask(const TaskFunc& f, const PeriodTime& tm);
    ~CTimePointTask(void);

public:
    bool is_started() const;
    bool start();
    void stop();

private:
    void worker_func();

private:
    bool m_started;

    TaskFunc m_f;
    const PeriodTime m_tm;

    boost::thread m_worker_thread;
    HANDLE m_hExitEvent;
};
