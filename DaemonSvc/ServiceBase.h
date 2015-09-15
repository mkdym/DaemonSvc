#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include "../../Include/tstd.h"



class CServiceBase : public boost::noncopyable
{
public:
    typedef boost::function<void(int argc, tchar * argv[])> RunAction;
    typedef boost::function<void()> OtherAction;

    CServiceBase(const tstring& ServiceName,
        const tstring& DisplayName,
        const RunAction& action,
        const DWORD DesiredAccess = SERVICE_ALL_ACCESS,
        const DWORD ServiceType = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
        const DWORD StartType = SERVICE_AUTO_START,
        const DWORD ErrorControl = SERVICE_ERROR_NORMAL,
        const tchar* LoadOrderGroup = NULL,
        const DWORD TagId = 0,
        const tchar* Dependencies = NULL,
        const tchar* ServiceStartName = NULL,
        const tchar* Password = NULL,
        const DWORD ControlsAccepted = SERVICE_ACCEPT_STOP);

    ~CServiceBase(void);

public:
    BOOL ReportRunning(const DWORD nWaitHint)
    {
        return ReportStatus(SERVICE_RUNNING, nWaitHint);
    }

    BOOL ReportStopPending(const DWORD nWaitHint)
    {
        return ReportStatus(SERVICE_STOP_PENDING, nWaitHint);
    }

    void AddControlCodeAction(const DWORD dwCtrlCode, const OtherAction& action)
    {
        m_Actions[dwCtrlCode] = action;
    }

    void SetStopAction(const OtherAction& action)
    {
        AddControlCodeAction(SERVICE_CONTROL_STOP, action);
    }

    void SetPauseAction(const OtherAction& action)
    {
        AddControlCodeAction(SERVICE_CONTROL_PAUSE, action);
    }

    void SetContinueAction(const OtherAction& action)
    {
        AddControlCodeAction(SERVICE_CONTROL_CONTINUE, action);
    }

    void SetShutdownAction(const OtherAction& action)
    {
        AddControlCodeAction(SERVICE_CONTROL_SHUTDOWN, action);
    }

public:
    void SetInServiceMode(const bool bInServiceMode)
    {
        m_bInServiceMode = bInServiceMode;
    }

    typedef std::vector<tstring> ArgList;
    BOOL RegisterService(const ArgList& args);

private:
    BOOL ReportStatus(const DWORD nState, const DWORD nWaitHint);
    void ServiceCtrl(const DWORD dwCtrlCode);
    BOOL StartDispatcher();
    static void WINAPI ServiceCtrlEntry(DWORD dwCtrlCode);
    static BOOL WINAPI ConsoleCtrlHandlerRoutine(DWORD dwCtrlType);
    static void WINAPI ServiceMain(int argc, tchar * argv[]);

    BOOL InstallService();
    BOOL RemoveService();
    BOOL StartupService();
    BOOL StopService();

private:
    //服务的特性
    const tstring m_ServiceName;
    const tstring m_DisplayName;
    const RunAction m_RunAction;
    const DWORD m_DesiredAccess;
    const DWORD m_ServiceType;
    const DWORD m_StartType;
    const DWORD m_ErrorControl;
    const tchar* m_LoadOrderGroup;
    const DWORD m_TagId;
    const tchar* m_Dependencies;
    const tchar* m_ServiceStartName;
    const tchar* m_Password;

    DWORD m_ControlsAccepted;//可接受的控制指令
    bool m_bInServiceMode;//是否是以服务方式运行的。当非服务方式运行时，ReportStatus不作为
    SERVICE_STATUS_HANDLE m_hServiceStatus;//服务状态句柄
    SERVICE_STATUS m_Status;//服务状态

    typedef std::map<DWORD, OtherAction> ActionMap;
    ActionMap m_Actions;
};
