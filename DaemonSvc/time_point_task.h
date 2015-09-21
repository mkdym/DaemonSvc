#pragma once
#include "task_common.h"
#include "task_base.h"


//时间点任务：在每天、每周、每月的指定时间执行
class CTimePointTask : public CTaskBase
{
public:
    CTimePointTask(const TaskFunc& f, const PERIOD_TYPE& type, const TaskTime& tm);
    ~CTimePointTask(void);

public:
    bool is_started() const;
    bool start();
    void stop();

private:
    bool m_started;
};
