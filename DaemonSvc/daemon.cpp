#include "logger.h"
#include "single_checker.h"
#include "tasks_holder.h"
#include "config_loader.h"
#include "daemon.h"

CDaemon::CDaemon(void)
{
}

CDaemon::~CDaemon(void)
{
}

bool CDaemon::start()
{
    InfoLog("start begin");
    bool bReturn = false;

    do 
    {
        if (!CSingleChecker::get_instance_ref().single(TSTR("{3387415F-A686-4692-AA54-3A16AAEF9D5C}")))
        {
            ErrorLog("app already running");
            break;
        }

        if (!start_tasks_by_config(TSTR("")))
        {
            break;
        }

        m_exit_event.reset(CreateEvent(NULL, TRUE, FALSE, NULL));
        if (!m_exit_event.valid())
        {
            ErrorLogLastErr(CLastErrorFormat(), "CreateEvent fail");
            break;
        }

        bReturn = true;

    } while (false);

    InfoLog("start end");
    return bReturn;
}

void CDaemon::keep_running()
{
    InfoLog("keep_running begin");
    const DWORD r = WaitForSingleObject(m_exit_event.get(), INFINITE);
    switch (r)
    {
    case WAIT_OBJECT_0:
        InfoLog("got exit notify");
        break;

    default:
        ErrorLogLastErr(CLastErrorFormat(), "WaitForSingleObject fail, return code: %lu", r);
        break;
    }
    InfoLog("keep_running end");
}

void CDaemon::stop()
{
    InfoLog("stop begin");
    //和start保持一致，都是remove_all
    CTasksHolder::get_instance_ref().stop_all();
    CTasksHolder::get_instance_ref().delete_all();
    SetEvent(m_exit_event.get());
    InfoLog("stop end");
}

void CDaemon::restart()
{
    InfoLog("restart begin");
    start_tasks_by_config(TSTR(""));
    InfoLog("restart end");
}

bool CDaemon::start_tasks_by_config(const tstring& config_file)
{
    CTasksHolder::get_instance_ref().stop_all();
    CTasksHolder::get_instance_ref().delete_all();
    CConfigLoader cfg(config_file);

    {
        CConfigLoader::ti_info_list infos;
        cfg.get(infos);

        for (CConfigLoader::ti_info_list::const_iterator iter_info = infos.begin();
            iter_info != infos.end();
            ++iter_info)
        {
            CTasksHolder::get_instance_ref().add_time_interval_task(boost::bind(cmd_run_as,
                iter_info->common_info.cmd, iter_info->common_info.run_as, iter_info->common_info.show_window),
                iter_info->interval_seconds);
        }
    }

    {
        CConfigLoader::tp_info_list infos;
        cfg.get(infos);

        for (CConfigLoader::tp_info_list::const_iterator iter_info = infos.begin();
            iter_info != infos.end();
            ++iter_info)
        {
            CTasksHolder::get_instance_ref().add_time_point_task(boost::bind(cmd_run_as,
                iter_info->common_info.cmd, iter_info->common_info.run_as, iter_info->common_info.show_window),
                iter_info->pt);
        }
    }

    {
        CConfigLoader::pne_info_list infos;
        cfg.get(infos);

        for (CConfigLoader::pne_info_list::const_iterator iter_info = infos.begin();
            iter_info != infos.end();
            ++iter_info)
        {
            CTasksHolder::get_instance_ref().add_proc_non_exist_task(boost::bind(cmd_run_as,
                iter_info->common_info.cmd, iter_info->common_info.run_as, iter_info->common_info.show_window),
                iter_info->proc_path, iter_info->interval_seconds);
        }
    }

    std::vector<CTasksHolder::TaskId> failed_ids;
    CTasksHolder::get_instance_ref().start_all(failed_ids);
    if (!failed_ids.empty())
    {
        ErrorLog("start tasks fail");
        return false;
    }
    else
    {
        return true;
    }
}

