#include "../../Include/Tools/ServiceTools.h"
#include "ServiceBase.h"


static CServiceBase *g_service = NULL;//全局的静态服务实例


CServiceBase::CServiceBase(const tstring& ServiceName,
                           const tstring& DisplayName,
                           const RunAction& action,
                           const DWORD DesiredAccess /*= SERVICE_ALL_ACCESS*/,
                           const DWORD ServiceType /*= SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS*/,
                           const DWORD StartType /*= SERVICE_AUTO_START*/,
                           const DWORD ErrorControl /*= SERVICE_ERROR_NORMAL*/,
                           const tchar* LoadOrderGroup /*= NULL*/,
                           const DWORD TagId /*= 0*/,
                           const tchar* Dependencies /*= NULL*/,
                           const tchar* ServiceStartName /*= NULL*/,
                           const tchar* Password /*= NULL*/,
                           const DWORD ControlsAccepted /*= SERVICE_ACCEPT_STOP*/)

                           : m_ServiceName(ServiceName)
                           , m_DisplayName(DisplayName)
                           , m_RunAction(action)

                           , m_DesiredAccess(DesiredAccess)
                           , m_ServiceType(ServiceType)
                           , m_StartType(StartType)
                           , m_ErrorControl(ErrorControl)
                           , m_LoadOrderGroup(LoadOrderGroup)
                           , m_TagId(TagId)
                           , m_Dependencies(Dependencies)
                           , m_ServiceStartName(ServiceStartName)
                           , m_Password(Password)

                           , m_ControlsAccepted(ControlsAccepted)
                           , m_bInServiceMode(true)
                           , m_hServiceStatus(NULL)
{
    g_service = this;
    if (!SetConsoleCtrlHandler(ConsoleCtrlHandlerRoutine, TRUE))
    {
        ErrorLog(TEXT("SetConsoleCtrlHandler fail, error code: %d"), GetLastError());
    }
}

CServiceBase::~CServiceBase(void)
{
}


void CServiceBase::ServiceCtrl(const DWORD dwCtrlCode)
{
    switch(dwCtrlCode)
    {
    case SERVICE_CONTROL_STOP:
        m_Status.dwCurrentState = SERVICE_STOP_PENDING;
        Stop();
        break;

    case SERVICE_CONTROL_PAUSE:
        m_Status.dwCurrentState = SERVICE_PAUSE_PENDING;
        Pause();
        break;

    case SERVICE_CONTROL_CONTINUE:
        m_Status.dwCurrentState = SERVICE_CONTINUE_PENDING;
        Continue();
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        //m_status.dwCurrentState = SERVICE_STOP_PENDING;
        Shutdown();
        break;

    case SERVICE_CONTROL_INTERROGATE:
        // Update the service status.
        ReportStatus(m_Status.dwCurrentState, 3000);
        break;

    default:
        ErrorLog(TEXT("unknown service control code: %d"), dwCtrlCode);
        break;
    }
}

BOOL CServiceBase::RegisterService(const ArgList& args)
{
    BOOL bReturn = FALSE;

    switch (args.size())
    {
    case 1:
        break;

    case 2:
        break;

    default:
        break;
    }

    //todo
    if (1 == argc)//无参数
    {
        bReturn = StartDispatcher();
    }
    else if (2 == argc)
    {
        if (0 == strcmp("-install", argv[1]))
        {
            bReturn = InstallService();
        }
        else if (0 == strcmp("-remove", argv[1]))
        {
            bReturn = RemoveService();
        }
        else if (0 == strcmp("-start", argv[1]))
        {
            bReturn = StartupService();
        }
        else if (0 == strcmp("-stop", argv[1]))
        {
            bReturn = StopService();
        }
        else
        {
            ErrorLog(TEXT("invalid argv[1]: %s"), argv[1]);
        }
    }
    else
    {
        ErrorLog(TEXT("invalid argc: %d"), argc);
    }
    
    return bReturn;
}

BOOL CServiceBase::ReportStatus(const DWORD nState, const DWORD nWaitHint)
{
    if (!m_bInServiceMode)
    {
        InfoLog(TEXT("ReportStatus: %d"), nState);
        return TRUE;
    }
    else
    {
        if (SERVICE_START_PENDING == nState)
        {
            m_Status.dwControlsAccepted = 0;
        }
        else
        {
            m_Status.dwControlsAccepted = m_ControlsAccepted;
        }

        m_Status.dwServiceType = m_ServiceType;
        m_Status.dwCurrentState = nState;
        m_Status.dwWin32ExitCode = NO_ERROR;
        m_Status.dwServiceSpecificExitCode = 0;
        m_Status.dwWaitHint = nWaitHint;

        static DWORD dwCheckPoint = 0;
        if (SERVICE_PAUSED == nState
            || SERVICE_RUNNING == nState
            || SERVICE_STOPPED == nState)
        {
            dwCheckPoint = 0;
        }
        else
        {
            ++dwCheckPoint;
        }
        m_Status.dwCheckPoint = dwCheckPoint;

        BOOL bSetOK = SetServiceStatus(m_hServiceStatus, &m_Status);
        if (!bSetOK)
        {
            ErrorLog(TEXT("SetServiceStatus fail, error code: %d"), GetLastError());
        }
        return bSetOK;
    }
}

void WINAPI CServiceBase::ServiceCtrlEntry(DWORD dwCtrlCode)
{
    g_service->ServiceCtrl(dwCtrlCode);
}

BOOL CServiceBase::StartDispatcher()
{
    TCHAR szServiceName[MAX_PATH] = {0};
    _tcscpy_s(szServiceName, MAX_PATH, m_lpServiceName);
    SERVICE_TABLE_ENTRY dispatchTable[] =
    {
        {szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
        {0, 0}
    };
    BOOL bReturn = StartServiceCtrlDispatcher(dispatchTable);
    if (!bReturn)
    {
        ErrorLog(TEXT("StartServiceCtrlDispatcher fail, error code: %d"), GetLastError());
    }
    return bReturn;
}

BOOL WINAPI CServiceBase::ConsoleCtrlHandlerRoutine(DWORD dwCtrlType)
{
    BOOL bProcessed;

    switch(dwCtrlType)
    {
    case CTRL_BREAK_EVENT:
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        InfoLog(TEXT("got console event: %d. stopping service"), dwCtrlType);
        if (g_service->m_Actions.count(SERVICE_CONTROL_STOP))
        {
            g_service->m_Actions[SERVICE_CONTROL_STOP]();
        }
        bProcessed = TRUE;
        break;

    default:
        bProcessed = FALSE;
        break;
    }

    return bProcessed;
}

//这个参数是StartService传过来的，所以用TCHAR
void WINAPI CServiceBase::ServiceMain(int argc, tchar * argv[])
{
    g_service->m_hServiceStatus = RegisterServiceCtrlHandler(g_service->m_ServiceName.c_str(), ServiceCtrlEntry);
    if (g_service->m_hServiceStatus)
    {
        if (g_service->ReportStatus(SERVICE_START_PENDING, 3000))
        {
            ArgList args;
            for (int ArgIndex = 0; ArgIndex != argc; ++ArgIndex)
            {
               args.push_back(argv[ArgIndex]);
            }
            g_service->m_RunAction(args);
        }

        if (g_service->m_hServiceStatus)
        {
            g_service->ReportStatus(SERVICE_STOPPED, 3000);
        }
    }
}

BOOL CServiceBase::InstallService()
{
    TCHAR szBinaryFile[1024] = {0};
    if (!GetModuleFileName(NULL, szBinaryFile, 1023))
    {
        ErrorLog(TEXT("GetModuleFileName fail, error code: %d"), GetLastError());
        return FALSE;
    }
    else
    {
        return Tools::InstallService(m_lpServiceName, m_lpDisplayName, szBinaryFile,
            m_ServiceType, m_StartType, m_ErrorControl, m_LoadOrderGroup, m_TagId,
            m_Dependencies, m_ServiceStartName, m_Password);
    }
}

BOOL CServiceBase::RemoveService()
{
    return Tools::RemoveService(m_lpServiceName);
}

BOOL CServiceBase::StartupService()
{
    return Tools::StartupService(m_lpServiceName, 30 * 1000, &m_Status);
}

BOOL CServiceBase::StopService()
{
    return Tools::StopService(m_lpServiceName, 30 * 1000, &m_Status);
}

BOOL CServiceBase::ReportRunning(const DWORD nWaitHint)
{
    return ReportStatus(SERVICE_RUNNING, nWaitHint);
}

BOOL CServiceBase::ReportStopPending(const DWORD nWaitHint)
{
    return ReportStatus(SERVICE_STOP_PENDING, nWaitHint);
}


