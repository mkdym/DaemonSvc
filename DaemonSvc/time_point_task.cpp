#include <cassert>
#include <Windows.h>
#include <boost/bind.hpp>
#include "logger.h"
#include "time_point_task.h"


CTimePointTask::CTimePointTask(const TaskFunc& f, const PeriodTime& tm)
    : m_started(false)
    , m_f(f)
    , m_tm(tm)
    , m_hExitEvent(NULL)
{
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
        assert(NULL == m_hExitEvent);

        m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (NULL == m_hExitEvent)
        {
            ErrorLogLastErr(CLastError(), TSTR("CreateEvent for notify time point task thread exit fail"));
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
                ErrorLogA("create time point task worker thread fail, error: %s", e.what());
            }
        }

        return m_started;
    }
}

void CTimePointTask::stop()
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

void CTimePointTask::worker_func()
{
    InfoLogA("time point task worker thread func begin");

    while (true)
    {
        SYSTEMTIME systime = {0};
        GetLocalTime(&systime);

        //todo
        const DWORD wait_result = WaitForSingleObject(m_hExitEvent, INFINITE);
        if (WAIT_OBJECT_0 == wait_result)
        {
            InfoLogA("got exit notify");
            break;
        }
    }

    InfoLogA("time point task worker thread func end");
}

