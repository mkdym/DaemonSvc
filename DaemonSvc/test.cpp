#include <Windows.h>
#include "Win32Service.h"



HANDLE g_exit_event = NULL;


bool starting(const CWin32Service::ArgList& args)
{
    g_exit_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == g_exit_event)
    {
        ErrorLogA("CreateEvent fail, error code: %d", GetLastError());
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
        ErrorLogA("WaitForSingleObject fail, return code: %d, error code: %d", r, GetLastError());
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
    InitLog(TEXT("C:\\"));

    ServiceInfo si;
    si.name = TEXT("TestService");
    si.display_name = si.name;

#if defined(DEBUG) || defined(_DEBUG)
    const bool service_mode = false;
#else
    const bool service_mode = true;
#endif

    if (!CWin32Service::GetInstanceRef().Init(si, service_mode))
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
    system("pause");
#endif

    return 0;
}

