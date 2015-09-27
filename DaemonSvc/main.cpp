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
    CDaemon::get_instance_ref().notify_stop();
}

void restart(const CWin32Service::ArgList& args)
{
    CDaemon::get_instance_ref().restart();
}



int main(int argc, char * argv[])
{
    InitLog(TSTR(""));
    WindowsUtil::set_privilege(SE_DEBUG_NAME, true);

    ServiceInfo si;
    si.name = TSTR("DaemonSvc");
    si.display_name = si.name;

    if (!CWin32Service::get_instance_ref().Init(si))
    {
        ErrorLogA("init service fail");
    }
    else
    {
        InfoLogA("init service success");

        CWin32Service::get_instance_ref().RegisterStartingFunction(starting);
        CWin32Service::get_instance_ref().RegisterRunningFunction(running);
        CWin32Service::get_instance_ref().RegisterControlCodeFunction(SERVICE_CONTROL_STOP, stopping);
        CWin32Service::get_instance_ref().RegisterControlCodeFunction(200, restart);

        if (!CWin32Service::get_instance_ref().Go())
        {
            ErrorLogA("make service go fail");
        }
        else
        {
            InfoLogA("everything is OK");
        }
    }

#if defined(DEBUG) || defined(_DEBUG)
    if (CWin32Service::get_instance_ref().GetMode() != CWin32Service::S_DISPATCH)
    {
        system("pause");
    }
#endif

    return 0;
}

