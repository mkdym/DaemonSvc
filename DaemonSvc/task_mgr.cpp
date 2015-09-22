#include <cassert>
#include <limits>
#include "logger.h"
#include "task_mgr.h"


CTaskMgr::CTaskMgr(void)
{

}

CTaskMgr::~CTaskMgr(void)
{

}

CTaskMgr::TaskId CTaskMgr::add_time_point_task(const TaskFunc& f, const PeriodTime& period, const CTimePointTask::DIFF_TYPE& diff_type, const DWORD diff_seconds)
{
    assert(!(0 == period.dayofmonth
        && 0 == period.dayofweek
        && 0 == period.hour
        && 0 == period.minute
        && 0 == period.second));

    const TaskId id = alloc_task_num_id();
    m_tasks[id] = TaskBasePtr(new CTimePointTask(f, period, diff_type, diff_seconds));
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




