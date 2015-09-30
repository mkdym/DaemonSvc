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
    InitLog("", 0);
    WindowsUtil::set_privilege(SE_DEBUG_NAME, true);

    ServiceInfo si;
    si.name = TSTR("DaemonSvc");
    si.display_name = si.name;

    if (!CWin32Service::get_instance_ref().init(si))
    {
        ErrorLog("init service fail");
    }
    else
    {
        InfoLog("init service success");

        CWin32Service::get_instance_ref().register_starting_function(starting);
        CWin32Service::get_instance_ref().register_running_function(running);
        CWin32Service::get_instance_ref().register_control_code_function(SERVICE_CONTROL_STOP, stopping);
        CWin32Service::get_instance_ref().register_control_code_function(200, restart);

        if (!CWin32Service::get_instance_ref().go())
        {
            ErrorLog("make service go fail");
        }
        else
        {
            InfoLog("everything is OK");
        }
    }

#if defined(DEBUG) || defined(_DEBUG)
    if (CWin32Service::get_instance_ref().get_mode() != CWin32Service::S_DISPATCH)
    {
        system("pause");
    }
#endif

    return 0;
}

