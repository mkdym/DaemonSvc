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
    unsigned short dayofmonth;//1-31
    unsigned short dayofweek;//Sunday is 0, 0-6
    unsigned short hour;//0-23
    unsigned short minute;//0-59
    unsigned short second;//0-59

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
    enum DIFF_TYPE
    {
        POSITIVE,
        NEGATIVE,
        BOTH,
    };

    //diff_type是误差符号，diff_seconds是误差秒数
    CTimePointTask(const TaskFunc& f, const PeriodTime& period, const DIFF_TYPE& diff_type, const DWORD diff_seconds);
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
    const PeriodTime m_period;
    const DIFF_TYPE m_diff_type;
    const DWORD m_diff_seconds;

    boost::thread m_worker_thread;
    HANDLE m_hExitEvent;
};
