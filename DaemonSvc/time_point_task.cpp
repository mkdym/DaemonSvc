#include "time_point_task.h"


CTimePointTask::CTimePointTask(const TaskFunc& f, const PERIOD_TYPE& type, const TaskTime& tm)
{
}

CTimePointTask::~CTimePointTask(void)
{
}

bool CTimePointTask::is_started() const
{
    return false;
}

bool CTimePointTask::start()
{
    return false;
}

void CTimePointTask::stop()
{

}

