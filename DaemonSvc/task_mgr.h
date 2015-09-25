#pragma once
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include "tdef.h"
#include "task_common.h"
#include "task_base.h"
#include "time_point_task.h"
#include "time_interval_task.h"
#include "proc_non_exist_task.h"
#include "config_mgr.h"


class CTaskMgr : public boost::noncopyable
{
private:
    CTaskMgr(void);
    ~CTaskMgr(void);

public:
    static CTaskMgr& GetInstanceRef()
    {
        static CTaskMgr instance;
        return instance;
    }

public:
    typedef unsigned int TaskId;

    //diff «ŒÛ≤Ó
    TaskId add_time_point_task(const TaskFunc& f, const PeriodTime& period, const DWORD deviation_minutes);
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

public:
    //************************************
    // brief:    execute command
    // name:     CTaskMgr::exec
    // param:    const tstring & command            command line
    // param:    const CMD_RUN_AS_TYPE & run_as     run_as type
    // param:    const bool show_window             SW_SHOWNORMAL if true, otherwise SW_HIDE. see ShowWindow in MSDN
    // return:   bool                               return true if has created any process successfully
    // remarks:  if AS_LOGON_USER, exec in all logon users context(one-by-one) until one is successful
    //           if AS_ALL_LOGON_USERS, exec in all logon users context whether or not successful
    //************************************
    static bool exec(const tstring& command, const CMD_RUN_AS_TYPE& run_as, const bool show_window = true);

private:
    typedef boost::shared_ptr<CTaskBase> TaskBasePtr;
    typedef std::map<TaskId, TaskBasePtr> TaskMap;
    TaskMap m_tasks;
    boost::mutex m_tasks_lock;
};









