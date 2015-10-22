#include <cassert>
#include <Windows.h>
#include <boost/bind.hpp>
#include "logger.h"
#include "time_point_task.h"


CTimePointTask::CTimePointTask(const TaskFunc& f, const PeriodTime& period)
    : m_started(false)
    , m_f(f)
    , m_period(period)
{
    memset(&m_last_execute_time, 0, sizeof(m_last_execute_time));
}

CTimePointTask::~CTimePointTask(void)
{
    if (m_started)
    {
        stop();
    }
}

bool CTimePointTask::is_started() const
{
    return m_started;
}

bool CTimePointTask::start()
{
    if (m_started)
    {
        return true;
    }
    else
    {
        m_exit_event.reset(CreateEvent(NULL, TRUE, FALSE, NULL));
        if (!m_exit_event.valid())
        {
            ErrorLogLastErr("CreateEvent for notify time point task thread exit fail");
        }
        else
        {
            try
            {
                m_worker_thread = boost::thread(boost::bind(&CTimePointTask::worker_func, this));
                m_started = true;
            }
            catch (boost::thread_resource_error& e)
            {
                ErrorLog("create time point task worker thread fail, error: %s", e.what());
            }
        }

        return m_started;
    }
}

void CTimePointTask::stop()
{
    if (m_started)
    {
        assert(m_exit_event.valid());

        SetEvent(m_exit_event.get_ref());
        if (m_worker_thread.joinable())
        {
            m_worker_thread.join();
        }

        m_started = false;
    }
}

void CTimePointTask::worker_func()
{
    InfoLog("time point task worker thread func begin");
    ErrorLog("time point task not implemented");

    while (true)
    {
        SYSTEMTIME systime = {0};
        GetLocalTime(&systime);

        //计算上一次应执行的时间
        //看当前时间和上一次应执行时间之间有没有被执行过，即上次执行时间是否在上一次应执行时间之后
        //若有，则进行下一次等待
        //若没有，则判断是否在误差范围内，即当前时间-上一次应执行时间 是否 <= 误差
        //若在，则执行，并更新上一次执行时间
        //若不在，则进行下一次等待

        //等待时间：等待误差时间的一半，若这一半大于10分钟，则以10分钟计

        //todo
        const DWORD wait_result = WaitForSingleObject(m_exit_event.get_ref(), INFINITE);
        if (WAIT_OBJECT_0 == wait_result)
        {
            InfoLog("got exit notify");
            break;
        }
    }

    InfoLog("time point task worker thread func end");
}

