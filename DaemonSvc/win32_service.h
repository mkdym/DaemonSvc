#pragma once
#include <vector>
#include <map>
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include "service_info.h"


class CWin32Service : public boost::noncopyable
{
private:
    CWin32Service(void);
    ~CWin32Service(void);

public:
    static CWin32Service& GetInstanceRef()
    {
        static CWin32Service instance;
        return instance;
    }

public:
    enum S_MODE
    {
        S_DISPATCH,
        S_INSTALL,
        S_REMOVE,
        S_START,
        S_STOP,
        S_NORMAL_APP,
    };

    bool Init(const ServiceInfo& info);

    S_MODE GetMode()
    {
        return m_mode;
    }

    typedef std::vector<tstring> ArgList;
    typedef boost::function<bool(const ArgList&)> StartingFunction;//ArgList是应用程序的命令行参数
    typedef boost::function<void(const ArgList&)> ServiceFunction;

    void RegisterStartingFunction(const StartingFunction& f)
    {
        m_startingfunc = f;
    }

    void RegisterRunningFunction(const ServiceFunction& f)
    {
        m_runningfunc = f;
    }

    void RegisterControlCodeFunction(const DWORD c, const ServiceFunction& f)
    {
        assert(SERVICE_CONTROL_INTERROGATE != c);
        m_ctrlfuncs[c] = f;
    }

    bool Go();

private:
    bool ReportStatus(const DWORD nState, const DWORD nWaitHintMS);
    void ServiceCtrl(const DWORD code);
    bool StartDispatcher();
    BOOL ConsoleCtrl(DWORD code);
    bool ServiceMain();

    static void WINAPI s_ServiceCtrl(DWORD code);
    static BOOL WINAPI s_ConsoleCtrl(DWORD code);
    static void WINAPI s_ServiceMain(int argc, tchar * argv[]);

private:
    bool m_init_success;
    ServiceInfo m_info;
    ArgList m_args;
    StartingFunction m_startingfunc;
    ServiceFunction m_runningfunc;

    S_MODE m_mode;

    typedef std::map<DWORD, ServiceFunction> CtrlFuncs;
    CtrlFuncs m_ctrlfuncs;

    SERVICE_STATUS_HANDLE m_service_status_handle;//服务状态句柄
    SERVICE_STATUS m_service_status;//服务状态
};


