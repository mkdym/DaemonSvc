#include <cassert>
#include <Windows.h>
#include <boost/bind.hpp>
#include "logger.h"
#include "time_interval_task.h"


CTimeIntervalTask::CTimeIntervalTask(const TaskFunc& f, const DWORD interval_seconds)
    : m_started(false)
    , m_f(f)
    , m_interval_seconds(interval_seconds)
    , m_hExitEvent(NULL)
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
        assert(NULL == m_hExitEvent);

        m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (NULL == m_hExitEvent)
        {
            ErrorLogLastErr(CLastError(), TSTR("CreateEvent for notify time interval task thread exit fail"));
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
                ErrorLogA("create time interval task worker thread fail, error: %s", e.what());
            }
        }

        return m_started;
    }
}

void CTimeIntervalTask::stop()
{
    if (m_started)
    {
        assert(m_hExitEvent);

        SetEvent(m_hExitEvent);
        if (m_worker_thread.joinable())
        {
            m_worker_thread.join();
        }
        CloseHandle(m_hExitEvent);
        m_hExitEvent = NULL;

        m_started = false;
    }
}

void CTimeIntervalTask::worker_func()
{
    InfoLogA("time interval task worker thread func begin");

    while (true)
    {
        const DWORD wait_result = WaitForSingleObject(m_hExitEvent, m_interval_seconds * 1000);
        if (WAIT_OBJECT_0 == wait_result)
        {
            InfoLogA("got exit notify");
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
                    ErrorLogA("execute time interval task function exception");
                }
            }
        }
        else
        {
            ErrorLogLastErr(CLastError(), TSTR("WaitForSingleObject fail, return code: %d"), wait_result);
            //sleep some while for recover from error state
            if (WAIT_OBJECT_0 == WaitForSingleObject(m_hExitEvent, 1000))
            {
                InfoLogA("got exit notify");
                break;
            }
        }
    }

    InfoLogA("time interval task worker thread func end");
}


