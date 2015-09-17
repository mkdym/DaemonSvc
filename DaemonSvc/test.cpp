#include "Win32Service.h"



bool starting(const CWin32Service::ArgList& args)
{
    system("pause");
    return true;
}

void running(const CWin32Service::ArgList& args)
{
    system("pause");
}

void stopping(const CWin32Service::ArgList& args)
{
    system("pause");
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

    system("pause");
    return 0;
}

