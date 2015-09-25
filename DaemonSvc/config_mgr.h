#pragma once
#include <vector>
#include <boost/noncopyable.hpp>
#include "tdef.h"
#include "config_info.h"


class CConfigMgr : public boost::noncopyable
{
public:
    CConfigMgr(void);
    ~CConfigMgr(void);

public:
    void load(const tstring& file_path);

    typedef std::vector<TimeIntervalTaskInfo> time_interval_task_info_list;
    typedef std::vector<TimePointTaskInfo> time_point_task_info_list;
    typedef std::vector<ProcNonExistTaskInfo> proc_non_exist_task_info_list;

    void get(time_interval_task_info_list& infos) const;
    void get(time_point_task_info_list& infos) const;
    void get(proc_non_exist_task_info_list& infos) const;

private:
    time_interval_task_info_list m_time_interval_tasks_info;
    time_point_task_info_list m_time_point_tasks_info;
    proc_non_exist_task_info_list m_proc_non_exist_tasks_info;
};
