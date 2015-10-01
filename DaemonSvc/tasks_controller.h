#pragma once
#include <map>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include "singleton.h"
#include "tdef.h"
#include "period_time.h"
#include "task_base.h"


class CTasksController : public Singleton<CTasksController>
{
    friend class Singleton<CTasksController>;

private:
    CTasksController(void);

public:
    ~CTasksController(void);

public:
    typedef unsigned int TaskId;

    //diff «ŒÛ≤Ó
    TaskId add_time_point_task(const TaskFunc& f, const PeriodTime& period);
    TaskId add_time_interval_task(const TaskFunc& f, const DWORD interval_seconds);
    TaskId add_proc_non_exist_task(const TaskFunc& f, const tstring& proc_path, const DWORD interval_seconds);

    bool start_one(const TaskId id);
    void start_all(std::vector<TaskId>& failed_ids);

    void stop_one(const TaskId id);
    void stop_all();

    void delete_one(const TaskId id);
    void delete_all();

private:
    //…œÀ¯
    TaskId alloc_task_num_id();

private:
    typedef boost::shared_ptr<CTaskBase> TaskBasePtr;
    typedef std::map<TaskId, TaskBasePtr> TaskMap;
    TaskMap m_tasks;
    boost::mutex m_tasks_lock;
};









