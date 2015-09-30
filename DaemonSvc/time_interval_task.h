#pragma once
#include <Windows.h>
#include <boost/thread.hpp>
#include "scoped_handle.h"
#include "task_base.h"


//时间间隔任务：每间隔指定时间执行
class CTimeIntervalTask : public CTaskBase
{
public:
    CTimeIntervalTask(const TaskFunc& f, const DWORD interval_seconds);
    ~CTimeIntervalTask(void);

public:
    bool is_started() const;
    bool start();
    void stop();

private:
    void worker_func();

private:
    bool m_started;

    TaskFunc m_f;
    const DWORD m_interval_seconds;

    boost::thread m_worker_thread;
    scoped_handle<false> m_exit_event;
};
