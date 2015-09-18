#include <Windows.h>
#include "Win32Service.h"



HANDLE g_exit_event = NULL;


bool starting(const CWin32Service::ArgList& args)
{
    g_exit_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == g_exit_event)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void running(const CWin32Service::ArgList& args)
{
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
}

void stopping(const CWin32Service::ArgList& args)
{
    SetEvent(g_exit_event);
}



int main(int argc, char * argv[])
{
    ServiceInfo si;
    si.name = TEXT("TestService");
    si.display_name = si.name;
    if (!CWin32Service::GetInstanceRef().Init(si, false))
    {
        ErrorLogA("init service fail");
    }
    else
    {
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
    system("pause");
    return 0;
}

