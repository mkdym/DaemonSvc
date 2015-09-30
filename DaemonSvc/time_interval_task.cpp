#include <cassert>
#include <Windows.h>
#include <boost/bind.hpp>
#include "logger.h"
#include "time_interval_task.h"


CTimeIntervalTask::CTimeIntervalTask(const TaskFunc& f, const DWORD interval_seconds)
    : m_started(false)
    , m_f(f)
    , m_interval_seconds(interval_seconds)
{
}

CTimeIntervalTask::~CTimeIntervalTask(void)
{
    if (m_started)
    {
        stop();
    }
}

bool CTimeIntervalTask::is_started() const
{
    return m_started;
}

bool CTimeIntervalTask::start()
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
            ErrorLogLastErr(CLastErrorFormat(), "CreateEvent for notify time interval task thread exit fail");
        }
        else
        {
            try
            {
                m_worker_thread = boost::thread(boost::bind(&CTimeIntervalTask::worker_func, this));
                m_started = true;
            }
            catch (boost::thread_resource_error& e)
            {
                ErrorLog("create time interval task worker thread fail, error: %s", e.what());
            }
        }

        return m_started;
    }
}

void CTimeIntervalTask::stop()
{
    if (m_started)
    {
        assert(m_exit_event.valid());

        SetEvent(m_exit_event.get());
        if (m_worker_thread.joinable())
        {
            m_worker_thread.join();
        }

        m_started = false;
    }
}

void CTimeIntervalTask::worker_func()
{
    InfoLog("time interval task worker thread func begin");

    while (true)
    {
        const DWORD wait_result = WaitForSingleObject(m_exit_event.get(), m_interval_seconds * 1000);
        if (WAIT_OBJECT_0 == wait_result)
        {
            InfoLog("got exit notify");
            break;
        }

        if (WAIT_TIMEOUT == wait_result)
        {
            if (m_f)
            {
                try
                {
                    m_f();
                }
                catch (...)
                {
                    ErrorLog("execute time interval task function exception");
                }
            }
        }
        else
        {
            ErrorLogLastErr(CLastErrorFormat(), "WaitForSingleObject fail, return code: %lu", wait_result);
            //sleep some while for recover from error state
            if (WAIT_OBJECT_0 == WaitForSingleObject(m_exit_event.get(), 1000))
            {
                InfoLog("got exit notify");
                break;
            }
        }
    }

    InfoLog("time interval task worker thread func end");
}


