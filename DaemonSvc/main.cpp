#include <boost/bind.hpp>
#include "logger.h"
#include "daemon.h"
#include "win32_service.h"



bool starting(const CWin32Service::ArgList& args)
{
    return CDaemon::GetInstanceRef().start();
}

void running(const CWin32Service::ArgList& args)
{
    CDaemon::GetInstanceRef().keep_running();
}

void stopping(const CWin32Service::ArgList& args)
{
    CDaemon::GetInstanceRef().notify_stop();
}

void restart(const CWin32Service::ArgList& args)
{
    CDaemon::GetInstanceRef().restart();
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
        CWin32Service::GetInstanceRef().RegisterControlCodeFunction(200, restart);

        if (!CWin32Service::GetInstanceRef().Go())
        {
            ErrorLogA("make service go fail");
        }
        else
        {
            InfoLogA("everything is OK");
        }
    }

#if defined(DEBUG) || defined(_DEBUG)
    if (CWin32Service::GetInstanceRef().GetMode() != CWin32Service::S_DISPATCH)
    {
        system("pause");
    }
#endif

    return 0;
}

