#include <boost/bind.hpp>
#include "self_path.h"
#include "logger.h"
#include "single_checker.h"
#include "win32_service.h"
#include "task_mgr.h"
#include "config_loader.h"



HANDLE g_exit_event = NULL;


bool prepare_tasks()
{
    CConfigLoader cfg(TSTR(""));

    {
        CConfigLoader::time_interval_task_info_list infos;
        cfg.get(infos);

        for (CConfigLoader::time_interval_task_info_list::const_iterator iter_info = infos.begin();
            iter_info != infos.end();
            ++iter_info)
        {
            CTaskMgr::GetInstanceRef().add_time_interval_task(boost::bind(cmd_run_as,
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
            CTaskMgr::GetInstanceRef().add_time_point_task(boost::bind(cmd_run_as,
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
            CTaskMgr::GetInstanceRef().add_proc_non_exist_task(boost::bind(cmd_run_as,
                iter_info->cmd, iter_info->run_as, iter_info->show_window),
                iter_info->proc_path, iter_info->interval_seconds);
        }
    }

    std::vector<CTaskMgr::TaskId> failed_ids;
    CTaskMgr::GetInstanceRef().start_all(failed_ids);
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


bool starting(const CWin32Service::ArgList& args)
{
    InfoLogA("starting begin");
    bool bReturn = false;

    do 
    {
        if (!CSingleChecker::GetInstanceRef().single(TSTR("{3387415F-A686-4692-AA54-3A16AAEF9D5C}")))
        {
            ErrorLogA("app already running");
            break;
        }

        if (!prepare_tasks())
        {
            break;
        }

        g_exit_event = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (NULL == g_exit_event)
        {
            ErrorLogLastErr(CLastError(), TSTR("CreateEvent fail"));
            break;
        }

        bReturn = true;

    } while (false);

    InfoLogA("starting end");
    return bReturn;
}

void running(const CWin32Service::ArgList& args)
{
    InfoLogA("running begin");
    const DWORD r = WaitForSingleObject(g_exit_event, INFINITE);
    switch (r)
    {
    case WAIT_OBJECT_0:
        InfoLogA("got exit notify");
        break;

    default:
        ErrorLogLastErr(CLastError(), TSTR("WaitForSingleObject fail, return code: %lu"), r);
        break;
    }
    InfoLogA("running end");
}

void stopping(const CWin32Service::ArgList& args)
{
    InfoLogA("stoppping");
    SetEvent(g_exit_event);
}



int main(int argc, char * argv[])
{
    InitLog(TSTR(""));

    ServiceInfo si;
    si.name = TSTR("DaemonSvc");
    si.display_name = si.name;

    if (!CWin32Service::GetInstanceRef().Init(si))
    {
        ErrorLogA("init service fail");
    }
    else
    {
        InfoLogA("init service success");

        CWin32Service::GetInstanceRef().RegisterStartingFunction(starting);
        CWin32Service::GetInstanceRef().RegisterRunningFunction(running);
        CWin32Service::GetInstanceRef().RegisterControlCodeFunction(SERVICE_CONTROL_STOP, stopping);

        if (!CWin32Service::GetInstanceRef().Go())
        {
            ErrorLogA("make service go fail");
        }
        else
        {
            InfoLogA("everything is OK");
        }
    }

    if (g_exit_event)
    {
        CloseHandle(g_exit_event);
        g_exit_event = NULL;
    }

#if defined(DEBUG) || defined(_DEBUG)
    if (CWin32Service::GetInstanceRef().GetMode() != CWin32Service::S_DISPATCH)
    {
        system("pause");
    }
#endif

    return 0;
}

