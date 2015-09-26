#pragma once
#include <Windows.h>
#include <boost/thread.hpp>
#include "task_base.h"
#include "period_time.h"



//时间点任务：在每天、每周、每月的指定时间执行
class CTimePointTask : public CTaskBase
{
public:
    //deviation_minutes是误差
    CTimePointTask(const TaskFunc& f, const PeriodTime& period);
    ~CTimePointTask(void);

public:
    bool is_started() const;
    bool start();
    void stop();

private:
    void worker_func();

private:
    bool m_started;
    SYSTEMTIME m_last_execute_time;

    TaskFunc m_f;
    const PeriodTime m_period;

    boost::thread m_worker_thread;
    HANDLE m_hExitEvent;
};
