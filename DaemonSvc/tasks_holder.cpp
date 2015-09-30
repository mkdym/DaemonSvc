#include <cassert>
#include <limits>
#include "logger.h"
#include "time_point_task.h"
#include "time_interval_task.h"
#include "proc_non_exist_task.h"
#include "tasks_holder.h"


CTasksHolder::CTasksHolder(void)
{

}

CTasksHolder::~CTasksHolder(void)
{

}

CTasksHolder::TaskId CTasksHolder::add_time_point_task(const TaskFunc& f, const PeriodTime& period)
{
    assert(period.valid());

    const TaskId id = alloc_task_num_id();
    m_tasks[id] = TaskBasePtr(new CTimePointTask(f, period));
    InfoLog("added a time point task, id: %lu", id);
    return id;
}

CTasksHolder::TaskId CTasksHolder::add_time_interval_task(const TaskFunc& f, const DWORD interval_seconds)
{
    assert(interval_seconds);

    const TaskId id = alloc_task_num_id();
    m_tasks[id] = TaskBasePtr(new CTimeIntervalTask(f, interval_seconds));
    InfoLog("added a time interval task, id: %lu", id);
    return id;
}

CTasksHolder::TaskId CTasksHolder::add_proc_non_exist_task(const TaskFunc& f, const tstring& proc_path, const DWORD interval_seconds)
{
    assert(!proc_path.empty());
    assert(interval_seconds);

    const TaskId id = alloc_task_num_id();
    m_tasks[id] = TaskBasePtr(new CProcNonExistTask(f, proc_path, interval_seconds));
    InfoLog("added a proc non exist task, id: %lu", id);
    return id;
}

bool CTasksHolder::start_one(const TaskId id)
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
        ErrorLog("can not find task, id: %lu", id);
        return false;
    }
}

void CTasksHolder::start_all(std::vector<TaskId>& failed_ids)
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

void CTasksHolder::stop_one(const TaskId id)
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
        ErrorLog("can not find task, id: %lu", id);
    }
}

void CTasksHolder::stop_all()
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

void CTasksHolder::delete_one(const TaskId id)
{
    boost::lock_guard<boost::mutex> locker(m_tasks_lock);
    TaskMap::const_iterator it_task = m_tasks.find(id);
    if (it_task != m_tasks.end())
    {
        m_tasks.erase(it_task);
    }
}

void CTasksHolder::delete_all()
{
    boost::lock_guard<boost::mutex> locker(m_tasks_lock);
    m_tasks.clear();
}

CTasksHolder::TaskId CTasksHolder::alloc_task_num_id()
{
    //todo: are these two static vars have thread-safety problem???
    static TaskId id = 0;
    static boost::mutex id_lock;

    boost::lock_guard<boost::mutex> locker(id_lock);
    if (id > (std::numeric_limits<TaskId>::max)())
    {
        ErrorLog("task id is too large, you may hold too many tasks, id will restart from 0");
        id = 0;
    }
    else
    {
        ++id;
    }

    return id;
}




