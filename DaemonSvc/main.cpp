#include <boost/bind.hpp>
#include "logger.h"
#include "daemon.h"
#include "win32_service.h"
#include "windows_util.h"



bool starting(const CWin32Service::ArgList& args)
{
    return CDaemon::get_instance_ref().start();
}

void running(const CWin32Service::ArgList& args)
{
    CDaemon::get_instance_ref().keep_running();
}

void stopping(const CWin32Service::ArgList& args)
{
    CDaemon::get_instance_ref().stop();
}

void restart(const CWin32Service::ArgList& args)
{
    CDaemon::get_instance_ref().restart();
}



int main(int argc, char * argv[])
{
    InitLog("", 0, LOG_DEBUG);
    //try to enable debug privilege for querying other processes' info
    WindowsUtil::set_privilege(SE_DEBUG_NAME, true);

    ServiceInfo si;
    si.name = TSTR("DaemonSvc");
    si.display_name = si.name;

    CWin32Service& svc = CWin32Service::get_instance_ref();

    if (!svc.init(si))
    {
        ErrorLog("init service fail");
    }
    else
    {
        InfoLog("init service success");

        svc.register_starting_function(starting);
        svc.register_running_function(running);
        svc.register_control_code_function(SERVICE_CONTROL_STOP, stopping);
        svc.register_control_code_function(200, restart);

        if (!svc.go())
        {
            ErrorLog("make service go fail");
        }
        else
        {
            InfoLog("everything is OK");
        }
    }

#if defined(DEBUG) || defined(_DEBUG)
    if (svc.get_mode() != CWin32Service::S_DISPATCH)
    {
        system("pause");
    }
#endif

    return 0;
}

