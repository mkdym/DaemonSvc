#include <cassert>
#include <limits>
#include "logger.h"
#include "process_creator.h"
#include "process_scanner.h"
#include "task_mgr.h"


CTaskMgr::CTaskMgr(void)
{

}

CTaskMgr::~CTaskMgr(void)
{

}

CTaskMgr::TaskId CTaskMgr::add_time_point_task(const TaskFunc& f, const PeriodTime& period, const DWORD deviation_minutes)
{
    assert(!(0 == period.dayofmonth
        && 0 == period.dayofweek
        && 0 == period.hour
        && 0 == period.minute
        //&& 0 == period.second
        ));
    //todo: 在每种定时下，限制误差大小
    //每月任务：必须小于28天
    //每周任务：必须小于7天
    //每天任务：必须小于24小时

    const TaskId id = alloc_task_num_id();
    m_tasks[id] = TaskBasePtr(new CTimePointTask(f, period, deviation_minutes));
    InfoLogA("added a time point task, id: %d", id);
    return id;
}

CTaskMgr::TaskId CTaskMgr::add_time_interval_task(const TaskFunc& f, const DWORD interval_seconds)
{
    assert(interval_seconds);

    const TaskId id = alloc_task_num_id();
    m_tasks[id] = TaskBasePtr(new CTimeIntervalTask(f, interval_seconds));
    InfoLogA("added a time interval task, id: %d", id);
    return id;
}

CTaskMgr::TaskId CTaskMgr::add_proc_non_exist_task(const TaskFunc& f, const tstring& proc_path, const DWORD interval_seconds)
{
    assert(!proc_path.empty());
    assert(interval_seconds);

    const TaskId id = alloc_task_num_id();
    m_tasks[id] = TaskBasePtr(new CProcNonExistTask(f, proc_path, interval_seconds));
    InfoLogA("added a proc non exist task, id: %d", id);
    return id;
}

bool CTaskMgr::start_one(const TaskId id)
{
    TaskBasePtr ptask;
    {
        boost::lock_guard<boost::mutex> locker(m_tasks_lock);
        TaskMap::iterator it_task = m_tasks.find(id);
        if (it_task != m_tasks.end())
        {
            ptask = it_task->second;
        }
    }

    if (ptask)
    {
        if (ptask->is_started())
        {
            return true;
        }
        else
        {
            return ptask->start();
        }
    }
    else
    {
        ErrorLogA("can not find task, id: %d", id);
        return false;
    }
}

void CTaskMgr::start_all(std::vector<TaskId>& failed_ids)
{
    TaskMap tasks;
    {
        boost::lock_guard<boost::mutex> locker(m_tasks_lock);
        tasks = m_tasks;
    }

    for (TaskMap::iterator it_task = tasks.begin();
        it_task != tasks.end();
        ++it_task)
    {
        TaskBasePtr ptask = it_task->second;
        if (!ptask->is_started())
        {
            if (!ptask->start())
            {
                failed_ids.push_back(it_task->first);
            }
        }
    }
}

void CTaskMgr::stop_one(const TaskId id)
{
    TaskBasePtr ptask;
    {
        boost::lock_guard<boost::mutex> locker(m_tasks_lock);
        TaskMap::iterator it_task = m_tasks.find(id);
        if (it_task != m_tasks.end())
        {
            ptask = it_task->second;
        }
    }

    if (ptask)
    {
        if (ptask->is_started())
        {
            ptask->stop();
        }
    }
    else
    {
        ErrorLogA("can not find task, id: %d", id);
    }
}

void CTaskMgr::stop_all()
{
    TaskMap tasks;
    {
        boost::lock_guard<boost::mutex> locker(m_tasks_lock);
        tasks = m_tasks;
    }

    for (TaskMap::iterator it_task = tasks.begin();
        it_task != tasks.end();
        ++it_task)
    {
        TaskBasePtr ptask = it_task->second;
        if (ptask->is_started())
        {
            ptask->stop();
        }
    }
}

void CTaskMgr::delete_one(const TaskId id)
{
    boost::lock_guard<boost::mutex> locker(m_tasks_lock);
    TaskMap::const_iterator it_task = m_tasks.find(id);
    if (it_task != m_tasks.end())
    {
        m_tasks.erase(it_task);
    }
}

void CTaskMgr::delete_all()
{
    boost::lock_guard<boost::mutex> locker(m_tasks_lock);
    m_tasks.clear();
}

CTaskMgr::TaskId CTaskMgr::alloc_task_num_id()
{
    static TaskId id = 0;
    static boost::mutex id_lock;

    boost::lock_guard<boost::mutex> locker(id_lock);
    if (id > (std::numeric_limits<TaskId>::max)())
    {
        ErrorLogA("task id is too large, you may hold too many tasks, id will restart from 0");
        id = 0;
    }
    else
    {
        ++id;
    }

    return id;
}

bool CTaskMgr::exec(const tstring& command,
                    const RUN_AS_TYPE& run_as,
                    const bool show_window /*= true*/)
{
    InfoLogA("begin exec");

    bool execute_success = false;
    const DWORD sw_flag = show_window ? SW_SHOWNORMAL : SW_HIDE;

    std::vector<HANDLE> processes;

    switch (run_as)
    {
    case AS_LOCAL:
        {
            DWORD created_pid = 0;
            HANDLE hProcess = ProcessCreator::create_process_in_local_context(command, created_pid, CREATE_NEW_CONSOLE, TSTR(""), sw_flag);
            if (hProcess)
            {
                InfoLogA("create_process_in_local_context success, pid=%d", created_pid);
                processes.push_back(hProcess);
            }
            else
            {
                ErrorLogA("create_process_in_local_context fail");
            }
        }
        break;

    case AS_LOGON_USER:
    case AS_ALL_LOGON_USERS:
        {
            std::vector<DWORD> pids;
            CProcessScanner::find_pids_by_path(TSTR("explorer.exe"), pids);
            for (std::vector<DWORD>::const_iterator iter_pid = pids.begin();
                iter_pid != pids.end();
                ++iter_pid)
            {
                InfoLogA("explorer.exe pid=%d", *iter_pid);
                DWORD created_pid = 0;
                HANDLE hProcess = ProcessCreator::create_process_as_same_token(*iter_pid, command, created_pid, CREATE_NEW_CONSOLE, TSTR(""), sw_flag);
                if (hProcess)
                {
                    InfoLogA("create_process_as_same_token success, pid=%d", created_pid);
                    processes.push_back(hProcess);
                    if (run_as == AS_LOGON_USER)
                    {
                        break;
                    }
                }
                else
                {
                    ErrorLog(TSTR("create_process_as_same_token fail, pid=%d, cmd=[%s]"), *iter_pid, command.c_str());
                }
            }

            if (processes.empty())
            {
                ErrorLogA("no new process in user context was created, all fail, try create in local context");
                DWORD created_pid = 0;
                HANDLE hProcess = ProcessCreator::create_process_in_local_context(command, created_pid, CREATE_NEW_CONSOLE, TSTR(""), sw_flag);
                if (hProcess)
                {
                    InfoLogA("create_process_in_local_context success, pid=%d", created_pid);
                    processes.push_back(hProcess);
                }
                else
                {
                    ErrorLogA("create_process_in_local_context fail");
                }
            }
        }
        break;

    default:
        ErrorLogA("unknown run_as type: %d", run_as);
        break;
    }

    if (!processes.empty())
    {
        //最多伺候3秒，不必看结果，让守护进程去看
        WaitForMultipleObjects(processes.size(), &processes[0], TRUE, 3 * 1000);
        for (std::vector<HANDLE>::iterator iter_process = processes.begin();
            iter_process != processes.end();
            ++iter_process)
        {
            CloseHandle(*iter_process);
            *iter_process = NULL;
        }

        execute_success = true;
    }
    else
    {
        execute_success = false;
    }

    InfoLogA("end exec with %s", execute_success ? "success" : "fail");
    return execute_success;
}




