#include <cassert>
#include <Windows.h>
#include <boost/bind.hpp>
#include "logger.h"
#include "proc_non_exist_task.h"


CProcNonExistTask::CProcNonExistTask(const TaskFunc& f, const tstring& proc_path, const DWORD interval_seconds)
    : m_started(false)
    , m_f(f)
    , m_proc_path(proc_path)
    , m_interval_seconds(interval_seconds)
    , m_hExitEvent(NULL)
{
}

CProcNonExistTask::~CProcNonExistTask(void)
{
    if (m_started)
    {
        stop();
    }
}

bool CProcNonExistTask::is_started() const
{
    return m_started;
}

bool CProcNonExistTask::start()
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
            ErrorLogLastErr(CLastError(), TSTR("CreateEvent for notify proc non exist task thread exit fail"));
        }
        else
        {
            try
            {
                m_worker_thread = boost::thread(boost::bind(&CProcNonExistTask::worker_func, this));
                m_started = true;
            }
            catch (boost::thread_resource_error& e)
            {
                ErrorLogA("create proc non exist task worker thread fail, error: %s", e.what());
            }
        }

        return m_started;
    }
}

void CProcNonExistTask::stop()
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

void CProcNonExistTask::worker_func()
{
    InfoLogA("proc non exist task worker thread func begin");

    while (true)
    {
        Sleep(30 * 1000);
    }

    InfoLogA("proc non exist task worker thread func end");
}
