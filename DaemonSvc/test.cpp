#include <boost/bind.hpp>
#include "self_path.h"
#include "logger.h"
#include "single_checker.h"
#include "win32_service.h"
#include "task_mgr.h"
#include "cmd_run_as.h"
#include "config_mgr.h"



HANDLE g_exit_event = NULL;



void task_func(const tstring& hint)
{
    InfoLog(TSTR("task function: %s"), hint.c_str());
}




bool starting(const CWin32Service::ArgList& args)
{
    bool bReturn = false;

    do 
    {
        if (!CSingleChecker::GetInstanceRef().single(TSTR("{3387415F-A686-4692-AA54-3A16AAEF9D5C}")))
        {
            ErrorLogA("app already running");
            break;
        }

        CConfigMgr cfg;
        cfg.load(TSTR(""));

        {
            CConfigMgr::time_interval_task_info_list infos;
            cfg.get(infos);

            for (CConfigMgr::time_interval_task_info_list::const_iterator iter_info = infos.begin();
                iter_info != infos.end();
                ++iter_info)
            {
                CTaskMgr::GetInstanceRef().add_time_interval_task(boost::bind(cmd_run_as,
                    iter_info->cmd, iter_info->run_as, iter_info->show_window),
                    iter_info->interval_seconds);
            }
        }

        {
            CConfigMgr::time_point_task_info_list infos;
            cfg.get(infos);

            for (CConfigMgr::time_point_task_info_list::const_iterator iter_info = infos.begin();
                iter_info != infos.end();
                ++iter_info)
            {
                //todo
            }
        }

        {
            CConfigMgr::proc_non_exist_task_info_list infos;
            cfg.get(infos);

            for (CConfigMgr::proc_non_exist_task_info_list::const_iterator iter_info = infos.begin();
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
            ErrorLogA("start all tasks fail");
            break;
        }

        g_exit_event = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (NULL == g_exit_event)
        {
            ErrorLogLastErr(CLastError(), TSTR("CreateEvent fail"));
            break;
        }

        InfoLogA("starting OK");
        bReturn = true;

    } while (false);

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
        ErrorLogLastErr(CLastError(), TSTR("WaitForSingleObject fail, return code: %d"), r);
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

