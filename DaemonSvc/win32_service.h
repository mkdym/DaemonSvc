#pragma once
#include <vector>
#include <map>
#include <Windows.h>
#include <boost/function.hpp>
#include "singleton.h"
#include "service_info.h"


class CWin32Service : public Singleton<CWin32Service>
{
    friend class Singleton<CWin32Service>;

private:
    CWin32Service(void);

public:
    ~CWin32Service(void);

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

    bool init(const ServiceInfo& info);

    const S_MODE& get_mode() const
    {
        return m_mode;
    }

    typedef std::vector<tstring> ArgList;
    typedef boost::function<bool(const ArgList&)> StartingFunction;//ArgList是应用程序的命令行参数
    typedef boost::function<void(const ArgList&)> ServiceFunction;

    void register_starting_function(const StartingFunction& f)
    {
        m_startingfunc = f;
    }

    void register_running_function(const ServiceFunction& f)
    {
        m_runningfunc = f;
    }

    void register_control_code_function(const DWORD c, const ServiceFunction& f)
    {
        assert(SERVICE_CONTROL_INTERROGATE != c);
        m_ctrlfuncs[c] = f;
    }

    bool go();

private:
    bool report_status(const DWORD nState, const DWORD nWaitHintMS);
    void service_ctrl(const DWORD code);
    bool start_dispatcher();
    BOOL console_ctrl(DWORD code);
    bool service_main();

    static void WINAPI s_service_ctrl(DWORD code);
    static BOOL WINAPI s_console_ctrl(DWORD code);
    static void WINAPI s_service_main(int argc, tchar * argv[]);

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


