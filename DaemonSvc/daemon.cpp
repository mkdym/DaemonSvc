#include "logger.h"
#include "single_checker.h"
#include "tasks_holder.h"
#include "config_loader.h"
#include "daemon.h"

CDaemon::CDaemon(void)
    : m_exit_event(NULL)
{
}

CDaemon::~CDaemon(void)
{
    if (m_exit_event)
    {
        CloseHandle(m_exit_event);
    }
}

bool CDaemon::start()
{
    InfoLogA("start begin");
    bool bReturn = false;

    do 
    {
        if (!CSingleChecker::get_instance_ref().single(TSTR("{3387415F-A686-4692-AA54-3A16AAEF9D5C}")))
        {
            ErrorLogA("app already running");
            break;
        }

        if (!start_tasks_by_config(TSTR("")))
        {
            break;
        }

        if (m_exit_event)
        {
            CloseHandle(m_exit_event);
        }

        m_exit_event = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (NULL == m_exit_event)
        {
            ErrorLogLastErr(CLastError(), TSTR("CreateEvent fail"));
            break;
        }

        bReturn = true;

    } while (false);

    InfoLogA("start end");
    return bReturn;
}

void CDaemon::keep_running()
{
    InfoLogA("keep_running begin");
    const DWORD r = WaitForSingleObject(m_exit_event, INFINITE);
    switch (r)
    {
    case WAIT_OBJECT_0:
        InfoLogA("got exit notify");
        break;

    default:
        ErrorLogLastErr(CLastError(), TSTR("WaitForSingleObject fail, return code: %lu"), r);
        break;
    }
    InfoLogA("keep_running end");
}

void CDaemon::notify_stop()
{
    InfoLogA("notify_stop");
    SetEvent(m_exit_event);
}

void CDaemon::restart()
{
    InfoLogA("restart begin");
    start_tasks_by_config(TSTR(""));
    InfoLogA("restart end");
}

bool CDaemon::start_tasks_by_config(const tstring& config_file)
{
    CTasksHolder::get_instance_ref().stop_all();
    CTasksHolder::get_instance_ref().delete_all();
    CConfigLoader cfg(config_file);

    {
        CConfigLoader::time_interval_task_info_list infos;
        cfg.get(infos);

        for (CConfigLoader::time_interval_task_info_list::const_iterator iter_info = infos.begin();
            iter_info != infos.end();
            ++iter_info)
        {
            CTasksHolder::get_instance_ref().add_time_interval_task(boost::bind(cmd_run_as,
                iter_info->cmd, iter_info->run_as, iter_info->show_window),
                iter_info->interval_seconds);
        }
    }

    {
        CConfigLoader::time_point_task_info_list infos;
        cfg.get(infos);

        for (CConfigLoader::time_point_task_info_list::const_iterator iter_info = infos.begin();
            iter_info != infos.end();
            ++iter_info)
        {
            CTasksHolder::get_instance_ref().add_time_point_task(boost::bind(cmd_run_as,
                iter_info->cmd, iter_info->run_as, iter_info->show_window),
                iter_info->pt);
        }
    }

    {
        CConfigLoader::proc_non_exist_task_info_list infos;
        cfg.get(infos);

        for (CConfigLoader::proc_non_exist_task_info_list::const_iterator iter_info = infos.begin();
            iter_info != infos.end();
            ++iter_info)
        {
            CTasksHolder::get_instance_ref().add_proc_non_exist_task(boost::bind(cmd_run_as,
                iter_info->cmd, iter_info->run_as, iter_info->show_window),
                iter_info->proc_path, iter_info->interval_seconds);
        }
    }

    std::vector<CTasksHolder::TaskId> failed_ids;
    CTasksHolder::get_instance_ref().start_all(failed_ids);
    if (!failed_ids.empty())
    {
        ErrorLogA("start tasks fail");
        return false;
    }
    else
    {
        return true;
    }
}

