#include <cassert>
#include <Windows.h>
#include <boost/bind.hpp>
#include "logger.h"
#include "process_path_query.h"
#include "process_scanner.h"
#include "proc_non_exist_task.h"


CProcNonExistTask::CProcNonExistTask(const TaskFunc& f,
                                     const tstring& proc_path,
                                     const DWORD interval_seconds)
    : m_started(false)
    , m_f(f)
    , m_proc_path(proc_path)
    , m_interval_seconds(interval_seconds)
    , m_need_query_full_path(false)
{
    if (tstring::npos != m_proc_path.find_first_of(TSTR("\\/")))
    {
        m_need_query_full_path = true;
    }
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
        m_exit_event.reset(CreateEvent(NULL, TRUE, FALSE, NULL));
        if (!m_exit_event.valid())
        {
            ErrorLogLastErr("CreateEvent for notify proc non exist task thread exit fail");
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
                ErrorLog("create proc non exist task worker thread fail, error: %s", e.what());
            }
        }

        return m_started;
    }
}

void CProcNonExistTask::stop()
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

void CProcNonExistTask::worker_func()
{
    InfoLog("proc non exist task worker thread func begin");

    while (true)
    {
        std::vector<DWORD> pids;
        //todo: is exactly_match=false correct ?????
        find_pids_by_path(m_proc_path, pids, true, false);

        if (pids.empty())//non exist
        {
            InfoLog(TSTR("can not find process[%s], try to execute function if has"), m_proc_path.c_str());
            if (m_f)
            {
                try
                {
                    m_f();
                }
                catch (...)
                {
                    ErrorLog("execute proc non exist task function exception");
                }
            }

            //sleep some while if function has done something which will effect later on
            const DWORD wait_result = WaitForSingleObject(m_exit_event.get(), 1000);
            if (WAIT_OBJECT_0 == wait_result)
            {
                InfoLog("got exit notify");
                break;
            }
        }
        else//exist
        {
            const DWORD pid = pids.at(0);
            scoped_handle<> hProcess(OpenProcess(SYNCHRONIZE, FALSE, pid));
            if (!hProcess.valid())
            {
                ErrorLogLastErr("OpenProcess[%lu] fail", pid);

                const DWORD wait_result = WaitForSingleObject(m_exit_event.get(), m_interval_seconds * 1000);
                if (WAIT_OBJECT_0 == wait_result)
                {
                    InfoLog("got exit notify");
                    break;
                }
            }
            else
            {
                bool should_break = false;

                HANDLE pHandles[2] = {m_exit_event.get(), hProcess.get()};
                const DWORD wait_result = WaitForMultipleObjects(sizeof(pHandles) / sizeof(pHandles[0]),
                    pHandles, FALSE, INFINITE);
                switch (wait_result)
                {
                case WAIT_OBJECT_0:
                    InfoLog("got exit notify");
                    should_break = true;
                    break;

                case WAIT_OBJECT_0 + 1:
                    InfoLog(TSTR("process[%s] exited, try to execute function if has"), m_proc_path.c_str());
                    if (m_f)
                    {
                        try
                        {
                            m_f();
                        }
                        catch (...)
                        {
                            ErrorLog("execute proc non exist task function exception");
                        }
                    }
                    break;

                default:
                    ErrorLogLastErr("WaitForMultipleObjects fail, return code: %lu", wait_result);
                    //sleep some while for recover from error state
                    if (WAIT_OBJECT_0 == WaitForSingleObject(m_exit_event.get(), 1000))
                    {
                        InfoLog("got exit notify");
                        should_break = true;
                    }
                    break;
                }

                if (should_break)
                {
                    break;
                }
            }
        }
    }

    InfoLog("proc non exist task worker thread func end");
}

