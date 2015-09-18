#include <Windows.h>
#include <boost/algorithm/string.hpp>
#include <boost/smart_ptr.hpp>
#include "service_util.h"
#include "win32_service.h"


static const DWORD WAIT_HINT_MS = 30 * 1000;


CWin32Service::CWin32Service(void)
{
}

CWin32Service::~CWin32Service(void)
{
}

bool CWin32Service::Init(const ServiceInfo& info, const bool service_mode)
{
    m_info = info;
    m_service_mode = service_mode;

    m_args.clear();
    int arg_count = 0;
    LPWSTR *arg_str_list = CommandLineToArgvW(GetCommandLineW(), &arg_count);
    if (NULL == arg_str_list)
    {
        ErrorLogA("can not get commoand line, error code: %d", GetLastError());
        return false;
    }
    else
    {
        for(int arg_index = 0; arg_index != arg_count; ++arg_index)
        {
            m_args.push_back(arg_str_list[arg_index]);
        }

        if (!SetConsoleCtrlHandler(s_ConsoleCtrl, TRUE))
        {
            ErrorLogA("SetConsoleCtrlHandler fail, error code: %d", GetLastError());
        }

        LocalFree(arg_str_list);
        return true;
    }
}

bool CWin32Service::Go()
{
    bool bReturn = false;

    if (m_service_mode)
    {
        switch (m_args.size())
        {
        case 1://无参数
            bReturn = StartDispatcher();
            break;

        case 2:
            {
                tstring arg2 = m_args.at(1);
                boost::algorithm::trim(arg2);
                boost::algorithm::trim_if(arg2, boost::algorithm::is_any_of(TEXT("-/")));
                if (boost::algorithm::iequals(arg2, TEXT("install")))
                {
                    //install service
                    const DWORD len = 1024;
                    tchar szBinaryFile[len] = {0};
                    if (!GetModuleFileName(NULL, szBinaryFile, len - 1))
                    {
                        ErrorLogA("GetModuleFileName fail, error code: %d", GetLastError());
                        bReturn = false;
                    }
                    else
                    {
                        szBinaryFile[len - 1] = TEXT('\0');
                        bReturn = ServiceUtil::InstallService(m_info, szBinaryFile);
                    }
                }
                else if (boost::algorithm::iequals(arg2, TEXT("remove")))
                {
                    //remove service
                    bReturn = ServiceUtil::RemoveService(m_info.name);
                }
                else if (boost::algorithm::iequals(arg2, TEXT("start")))
                {
                    //start service
                    bReturn = ServiceUtil::StartupService(m_info.name, WAIT_HINT_MS);
                }
                else if (boost::algorithm::iequals(arg2, TEXT("stop")))
                {
                    //stop service
                    bReturn = ServiceUtil::StopService(m_info.name, WAIT_HINT_MS);
                }
                else
                {
                    ErrorLog(TEXT("invalid arg[1]: %s"), arg2);
                    bReturn = false;
                }
            }
            break;

        default:
            ErrorLogA("invalid argc: %d", m_args.size());
            bReturn = false;
            break;
        }
    }
    else//非服务模式，普通应用程序模式
    {
        bReturn = false;

        do 
        {
            if (m_startingfunc)
            {
                if (!m_startingfunc(m_args))
                {
                    ErrorLogA("call starting function fail");
                    break;
                }
            }

            if (m_runningfunc)
            {
                m_runningfunc(m_args);
            }

            bReturn = true;

        } while (false);
    }

    return bReturn;
}

bool CWin32Service::ReportStatus(const DWORD nState, const DWORD nWaitHintMS)
{
    if (!m_service_mode)
    {
        InfoLogA("ReportStatus: %d", nState);
        return true;
    }
    else
    {
        if (SERVICE_START_PENDING == nState)
        {
            m_service_status.dwControlsAccepted = 0;
        }
        else
        {
            m_service_status.dwControlsAccepted = m_info.accepted_controls;
        }

        m_service_status.dwServiceType = m_info.service_type;
        m_service_status.dwCurrentState = nState;
        m_service_status.dwWin32ExitCode = NO_ERROR;
        m_service_status.dwServiceSpecificExitCode = 0;
        m_service_status.dwWaitHint = nWaitHintMS;

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
        m_service_status.dwCheckPoint = dwCheckPoint;

        BOOL bReturn = SetServiceStatus(m_service_status_handle, &m_service_status);
        if (!bReturn)
        {
            ErrorLogA("SetServiceStatus fail when ReportStatus, error code: %d", GetLastError());
        }
        return (TRUE == bReturn);
    }
}

void CWin32Service::ServiceCtrl(const DWORD code)
{
    if (SERVICE_CONTROL_INTERROGATE == code)
    {
        //更新服务状态
        ReportStatus(m_service_status.dwCurrentState, WAIT_HINT_MS);
    }
    else
    {
        switch (code)
        {
        case SERVICE_CONTROL_STOP:
            m_service_status.dwCurrentState = SERVICE_STOP_PENDING;
            break;

        case SERVICE_CONTROL_PAUSE:
            m_service_status.dwCurrentState = SERVICE_PAUSE_PENDING;
            break;

        case SERVICE_CONTROL_CONTINUE:
            m_service_status.dwCurrentState = SERVICE_CONTINUE_PENDING;
            break;

        default:
            break;
        }

        CtrlFuncs::const_iterator it_func = m_ctrlfuncs.find(code);
        if (it_func != m_ctrlfuncs.end())
        {
            (it_func->second)(m_args);
        }
        else
        {
            ErrorLogA("unsupported service control code: %d", code);
        }
    }
}

bool CWin32Service::StartDispatcher()
{
    const DWORD name_len = m_info.name.size();
    boost::scoped_array<tchar> name(new tchar[name_len + 1]);
    memset(name.get(), 0, sizeof(tchar) * (name_len + 1));
    memcpy_s(name.get(), sizeof(tchar) * name_len, m_info.name.c_str(), sizeof(tchar) * name_len);

    const SERVICE_TABLE_ENTRY dispatchTable[] =
    {
        {name.get(), (LPSERVICE_MAIN_FUNCTION)s_ServiceMain},
        {0, 0}
    };

    BOOL bReturn = StartServiceCtrlDispatcher(dispatchTable);
    if (!bReturn)
    {
        ErrorLogA("StartServiceCtrlDispatcher fail, error code: %d", GetLastError());
    }

    return (TRUE == bReturn);
}

BOOL CWin32Service::ConsoleCtrl(DWORD code)
{
    BOOL bProcessed;

    switch(code)
    {
    case CTRL_BREAK_EVENT:
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        InfoLogA("got console stop event: %d", code);
        {
            CtrlFuncs::const_iterator it_func = m_ctrlfuncs.find(SERVICE_CONTROL_STOP);
            if (it_func != m_ctrlfuncs.end())
            {
                (it_func->second)(m_args);
            }
        }
        bProcessed = TRUE;
        break;

    default:
        bProcessed = FALSE;
        break;
    }

    return bProcessed;
}

void CWin32Service::ServiceMain(int argc, tchar * argv[])
{
    do 
    {
        m_service_status_handle = RegisterServiceCtrlHandler(m_info.name.c_str(), s_ServiceCtrl);
        if (NULL == m_service_status_handle)
        {
            ErrorLogA("RegisterServiceCtrlHandler fail, error code: %d", GetLastError());
            break;
        }

        if (!ReportStatus(SERVICE_START_PENDING, WAIT_HINT_MS))
        {
            ErrorLogA("report start_pending status fail");
            break;
        }

        if (m_startingfunc)
        {
            if (!m_startingfunc(m_args))
            {
                ErrorLogA("call starting function fail");
                break;
            }
        }

        if (!ReportStatus(SERVICE_RUNNING, WAIT_HINT_MS))
        {
            ErrorLogA("report running status fail");
            break;
        }

        if (m_runningfunc)
        {
            m_runningfunc(m_args);
        }

        if (!ReportStatus(SERVICE_STOPPED, WAIT_HINT_MS))
        {
            ErrorLogA("report running status fail");
            break;
        }

    } while (false);
}


void WINAPI CWin32Service::s_ServiceCtrl(DWORD code)
{
    CWin32Service::GetInstanceRef().ServiceCtrl(code);
}

BOOL WINAPI CWin32Service::s_ConsoleCtrl(DWORD code)
{
    return CWin32Service::GetInstanceRef().ConsoleCtrl(code);
}

void WINAPI CWin32Service::s_ServiceMain(int argc, tchar * argv[])
{
    return CWin32Service::GetInstanceRef().ServiceMain(argc, argv);
}


