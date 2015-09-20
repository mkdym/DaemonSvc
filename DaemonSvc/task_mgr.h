#include <time.h>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include "logger.h"


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
    typedef unsigned int TaskNumId;

    struct TaskId
    {
        TaskNumId num_id;
        time_t added_time;
        tstring hint;
    };

    enum TaskType
    {
        TIME_POINT,
        TIME_INTERVAL,
        PROC_NON_EXIST,
    };

    typedef boost::function<void()> TaskFunc;
    const TaskId& add(const TaskType& t, const TaskFunc& f, const tstring& hint);

    bool start_one(const TaskId& t);
    bool start_all();

    void stop_one(const TaskId& t);
    void stop_all();

    void delete_one(const TaskId& t);
    void delete_all();

private:
    //Ô­×Ó²Ù×÷
    TaskNumId alloc_task_num_id();

private:
    enum TaskStatus
    {
        TASK_ADDED,
        TASK_STARTED,
        TASK_STOPPED,
        TASK_DELETED,
    };

    struct TaskInfo
    {
        TaskId task_id;
        TaskType type;
        TaskFunc f;
        TaskStatus status;
    };

    std::map<TaskNumId, TaskInfo> m_tasks;
};









