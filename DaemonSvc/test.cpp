#include <Windows.h>
#include "logger.h"
#include "single_checker.h"
#include "win32_service.h"



HANDLE g_exit_event = NULL;


bool starting(const CWin32Service::ArgList& args)
{
    if (!CSingleChecker::GetInstanceRef().single(TEXT("{3387415F-A686-4692-AA54-3A16AAEF9D5C}")))
    {
        ErrorLogA("app already running");
        return false;
    }

    g_exit_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == g_exit_event)
    {
        ErrorLogLastErr(CLastError(), TEXT("CreateEvent fail"));
        return false;
    }
    else
    {
        InfoLogA("starting OK");
        return true;
    }
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
        ErrorLogLastErr(CLastError(), TEXT("WaitForSingleObject fail, return code: %d"), r);
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
    InitLog(TEXT(""));

    ServiceInfo si;
    si.name = TEXT("DaemonSvc");
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

