#include <cassert>
#include <boost/smart_ptr.hpp>
#include "boost_algorithm_string.h"
#include "self_path.h"
#include "logger.h"
#include "service_util.h"
#include "win32_service.h"


static const DWORD WAIT_HINT_MS = 30 * 1000;


static const tchar* console_event_str(const DWORD e)
{
    const tchar *p = TSTR("");
    switch (e)
    {
    case CTRL_C_EVENT:
        p = TSTR("CTRL_C_EVENT");
        break;

    case CTRL_BREAK_EVENT:
        p = TSTR("CTRL_BREAK_EVENT");
        break;

    case CTRL_CLOSE_EVENT:
        p = TSTR("CTRL_CLOSE_EVENT");
        break;

    case CTRL_LOGOFF_EVENT:
        p = TSTR("CTRL_LOGOFF_EVENT");
        break;

    case CTRL_SHUTDOWN_EVENT:
        p = TSTR("CTRL_SHUTDOWN_EVENT");
        break;

    default:
        p = TSTR("unknown??");
        break;
    }

    return p;
}



CWin32Service::CWin32Service(void)
    : m_init_success(false)
    , m_mode(S_NORMAL_APP)
    , m_service_status_handle(NULL)
{
}

CWin32Service::~CWin32Service(void)
{
}

bool CWin32Service::Init(const ServiceInfo& info)
{
    assert(!info.name.empty());

    m_init_success = false;
    m_info = info;
    m_args.clear();

    int arg_count = 0;
    LPWSTR *arg_str_list = CommandLineToArgvW(GetCommandLineW(), &arg_count);
    if (NULL == arg_str_list)
    {
        ErrorLogLastErr(CLastError(), TSTR("can not get commoand line"));
    }
    else
    {
        for(int arg_index = 0; arg_index != arg_count; ++arg_index)
        {
            m_args.push_back(arg_str_list[arg_index]);
        }
        LocalFree(arg_str_list);

        //parse commandline
        bool bValid = true;
        switch (m_args.size())
        {
        case 1:
            m_mode = S_NORMAL_APP;
            InfoLogA("normal app mode");
            break;

        case 2:
            {
                tstring arg2 = m_args.at(1);
                boost::algorithm::trim(arg2);
                boost::algorithm::trim_if(arg2, boost::algorithm::is_any_of(TSTR("-/")));

                if (boost::algorithm::iequals(arg2, TSTR("install")))
                {
                    m_mode = S_INSTALL;
                    InfoLogA("install service");
                }
                else if (boost::algorithm::iequals(arg2, TSTR("remove")))
                {
                    m_mode = S_REMOVE;
                    InfoLogA("remove service");
                }
                else if (boost::algorithm::iequals(arg2, TSTR("start")))
                {
                    m_mode = S_START;
                    InfoLogA("start service");
                }
                else if (boost::algorithm::iequals(arg2, TSTR("stop")))
                {
                    m_mode = S_STOP;
                    InfoLogA("stop service");
                }
                else if (boost::algorithm::iequals(arg2, TSTR("svc")))
                {
                    m_mode = S_DISPATCH;
                    InfoLogA("service mode");
                }
                else
                {
                    ErrorLog(TSTR("invalid arg[1]: %s"), arg2.c_str());
                    bValid = false;
                }
            }
            break;

        default:
            ErrorLogA("invalid argc: %lu", m_args.size());
            bValid = false;
            break;
        }

        if (bValid)
        {
            if (!SetConsoleCtrlHandler(s_ConsoleCtrl, TRUE))
            {
                ErrorLogLastErr(CLastError(), TSTR("SetConsoleCtrlHandler fail"));
            }
        }

        m_init_success = bValid;
    }

    return m_init_success;
}

bool CWin32Service::Go()
{
    if (!m_init_success)
    {
        ErrorLogA("must call init first");
        return false;
    }

    bool bReturn = false;

    switch (m_mode)
    {
    case S_DISPATCH:
        bReturn = StartDispatcher();
        break;

    case S_INSTALL:
        {
            tstring command = CSelfPath::GetInstanceRef().get_full();
            if (command.empty())
            {
                ErrorLogA("can not get full path name");
                bReturn = false;
            }
            else
            {
                command += TSTR(" -svc");
                InfoLog(TSTR("install command: %s"), command.c_str());
                bReturn = ServiceUtil::InstallService(m_info, command);
            }
        }
        break;

    case S_REMOVE:
        bReturn = ServiceUtil::RemoveService(m_info.name);
        break;

    case S_START:
        bReturn = ServiceUtil::StartupService(m_info.name, WAIT_HINT_MS);
        break;

    case S_STOP:
        bReturn = ServiceUtil::StopService(m_info.name, WAIT_HINT_MS);
        break;

    case S_NORMAL_APP:
        bReturn = ServiceMain();
        break;

    default:
        break;
    }

    return bReturn;
}

bool CWin32Service::ReportStatus(const DWORD nState, const DWORD nWaitHintMS)
{
    if (SERVICE_START_PENDING == nState)
    {
        //todo: 应该是忙的时候不允许其他动作，不止于“启动中”
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
        ErrorLogLastErr(CLastError(), TSTR("SetServiceStatus fail when ReportStatus"));
    }
    return (TRUE == bReturn);
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
            ErrorLogA("unsupported service control code: %lu", code);
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
        ErrorLogLastErr(CLastError(), TSTR("StartServiceCtrlDispatcher fail"));
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
        InfoLog(TSTR("got console stop event: %lu, %s"), code, console_event_str(code));
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

bool CWin32Service::ServiceMain()
{
    bool bReturn = false;

    do 
    {
        if (S_DISPATCH == m_mode)
        {
            m_service_status_handle = RegisterServiceCtrlHandler(m_info.name.c_str(), s_ServiceCtrl);
            if (NULL == m_service_status_handle)
            {
                ErrorLogLastErr(CLastError(), TSTR("RegisterServiceCtrlHandler fail"));
                break;
            }

            if (!ReportStatus(SERVICE_START_PENDING, WAIT_HINT_MS))
            {
                ErrorLogA("report start_pending status fail");
                break;
            }
        }

        if (m_startingfunc)
        {
            if (!m_startingfunc(m_args))
            {
                ErrorLogA("call starting function fail");
                break;
            }
        }

        if (S_DISPATCH == m_mode)
        {
            if (!ReportStatus(SERVICE_RUNNING, WAIT_HINT_MS))
            {
                ErrorLogA("report running status fail");
                break;
            }
        }

        if (m_runningfunc)
        {
            m_runningfunc(m_args);
        }

        if (S_DISPATCH == m_mode)
        {
            if (!ReportStatus(SERVICE_STOPPED, WAIT_HINT_MS))
            {
                ErrorLogA("report running status fail");
                break;
            }
        }

        bReturn = true;

    } while (false);

    return bReturn;
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
    CWin32Service::GetInstanceRef().ServiceMain();
}


