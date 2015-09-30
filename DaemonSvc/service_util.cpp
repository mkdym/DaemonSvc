#pragma once
#include <Windows.h>
#include <boost/smart_ptr.hpp>
#include "boost_algorithm_string.h"
#include "logger.h"
#include "service_util.h"


#pragma comment(lib, "Advapi32.lib")



static const tchar* service_status_str(const DWORD status)
{
    const tchar *p = TSTR("");
    switch (status)
    {
    case SERVICE_STOPPED:
        p = TSTR("SERVICE_STOPPED");
        break;

    case SERVICE_START_PENDING:
        p = TSTR("SERVICE_START_PENDING");
        break;

    case SERVICE_STOP_PENDING:
        p = TSTR("SERVICE_STOP_PENDING");
        break;

    case SERVICE_RUNNING:
        p = TSTR("SERVICE_RUNNING");
        break;

    case SERVICE_CONTINUE_PENDING:
        p = TSTR("SERVICE_CONTINUE_PENDING");
        break;

    case SERVICE_PAUSE_PENDING:
        p = TSTR("SERVICE_PAUSE_PENDING");
        break;

    case SERVICE_PAUSED:
        p = TSTR("SERVICE_PAUSED");
        break;

    default:
        p = TSTR("unknown??");
        break;
    }

    return p;
}


class scoped_scmgr_handle
{
public:
    scoped_scmgr_handle(const DWORD access)
        : m_h(NULL)
    {
        m_h = OpenSCManager(NULL, NULL, access);
        if (NULL == m_h)
        {
            ErrorLogLastErr(CLastErrorFormat(), "OpenSCManager fail");
        }
    }

    ~scoped_scmgr_handle()
    {
        if (m_h)
        {
            CloseServiceHandle(m_h);
            m_h = NULL;
        }
    }

public:
    bool valid() const
    {
        return m_h != NULL;
    }

    SC_HANDLE& get()
    {
        return m_h;
    }

private:
    SC_HANDLE m_h;
};


class scoped_svc_handle
{
public:
    scoped_svc_handle(const tstring& name, const DWORD sc_access, const DWORD svc_access)
        : m_sc(sc_access)
        , m_h(NULL)
    {
        if (m_sc.valid())
        {
            m_h = OpenService(m_sc.get(), name.c_str(), svc_access);
            if (NULL == m_h)
            {
                ErrorLogLastErr(CLastErrorFormat(), TSTR("OpenService[%s] fail"), name.c_str());
            }
        }
    }

    ~scoped_svc_handle()
    {
        if (m_h)
        {
            CloseServiceHandle(m_h);
            m_h = NULL;
        }
    }

public:
    bool valid() const
    {
        return m_h != NULL;
    }

    SC_HANDLE& get()
    {
        return m_h;
    }

private:
    scoped_scmgr_handle m_sc;
    SC_HANDLE m_h;
};



bool ServiceUtil::is_exist(const tstring& name)
{
    bool bReturn = false;

    do 
    {
        scoped_scmgr_handle hSCMgr(GENERIC_READ);
        if (!hSCMgr.valid())
        {
            break;
        }

        DWORD dwNeededBytes = 0;
        DWORD dwReturnedSerivice = 0;
        DWORD dwResumeEntryNum = 0;
        if (EnumServicesStatusEx(hSCMgr.get(), SC_ENUM_PROCESS_INFO, SERVICE_DRIVER | SERVICE_WIN32, 
            SERVICE_STATE_ALL, NULL, 0, &dwNeededBytes, &dwReturnedSerivice, &dwResumeEntryNum, NULL))
        {
            ErrorLog("EnumServicesStatusEx success while query needed bytes");
            break;
        }

        CLastErrorFormat e;
        if (ERROR_MORE_DATA != e.code())
        {
            ErrorLogLastErr(e, "EnumServicesStatusEx error code is not ERROR_MORE_DATA while query needed bytes");
            break;
        }

        boost::scoped_array<BYTE> lpData(new BYTE[dwNeededBytes]);
        memset(lpData.get(), 0, dwNeededBytes);
        if (!EnumServicesStatusEx(hSCMgr.get(), SC_ENUM_PROCESS_INFO, SERVICE_DRIVER | SERVICE_WIN32, 
            SERVICE_STATE_ALL, lpData.get(), dwNeededBytes, &dwNeededBytes, &dwReturnedSerivice, &dwResumeEntryNum, NULL))
        {
            ErrorLogLastErr(CLastErrorFormat(), "EnumServicesStatusEx fail");
            break;
        }

        ENUM_SERVICE_STATUS_PROCESS *lpServiceStatus = reinterpret_cast<ENUM_SERVICE_STATUS_PROCESS *>(lpData.get());
        for (DWORD dwServiceIndex = 0; dwServiceIndex != dwReturnedSerivice; ++dwServiceIndex)
        {
            tstring cur_name = lpServiceStatus[dwServiceIndex].lpServiceName;
            if (boost::algorithm::iequals(cur_name, name))
            {
                bReturn = true;
                break;
            }
        }

    } while (false);

    return bReturn;
}

bool ServiceUtil::is_running(const tstring& name)
{
    bool bReturn = false;

    do 
    {
        scoped_svc_handle hService(name, GENERIC_READ, GENERIC_READ);
        if (!hService.valid())
        {
            break;
        }

        SERVICE_STATUS status = {0};
        if (!QueryServiceStatus(hService.get(), &status))
        {
            ErrorLogLastErr(CLastErrorFormat(), TSTR("QueryServiceStatus[%s] fail"), name.c_str());
            break;
        }

        if (SERVICE_RUNNING != status.dwCurrentState)
        {
            DebugLog(TSTR("service[%s] is not running. status: %lu, %s"),
                name.c_str(), status.dwCurrentState, service_status_str(status.dwCurrentState));
            break;
        }

        bReturn = true;

    } while (false);

    return bReturn;
}

bool ServiceUtil::install(const ServiceInfo& info, const tstring& binary_file)
{
    bool bReturn = false;

    do 
    {
        scoped_scmgr_handle hSCMgr(GENERIC_ALL);
        if (!hSCMgr.valid())
        {
            break;
        }

        DWORD tag_id = info.tag_id;
        SC_HANDLE hService = CreateService(hSCMgr.get(),
            info.name.c_str(),
            info.display_name.c_str(),
            SERVICE_ALL_ACCESS,
            info.service_type,
            info.start_type,
            info.error_control,
            binary_file.c_str(),
            info.load_order_group.c_str(),
            info.use_tagid ? &tag_id : NULL,
            info.dependencies.c_str(),
            info.use_startname ? info.start_name.c_str() : NULL,
            info.use_password ? info.password.c_str() : NULL);
        if (NULL == hService)
        {
            ErrorLogLastErr(CLastErrorFormat(), TSTR("CreateService[%s] fail"), info.name.c_str());
            break;
        }

        CloseServiceHandle(hService);
        hService = NULL;

        bReturn = true;

    } while (false);

    return bReturn;
}

bool ServiceUtil::remove(const tstring& name)
{
    bool bReturn = false;

    do 
    {
        scoped_svc_handle hService(name, GENERIC_ALL, SERVICE_ALL_ACCESS);
        if (!hService.valid())
        {
            break;
        }

        if (!DeleteService(hService.get()))
        {
            ErrorLogLastErr(CLastErrorFormat(), TSTR("DeleteService[%s] fail"), name.c_str());
            break;
        }

        bReturn = true;

    } while (false);

    return bReturn;
}

bool ServiceUtil::startup(const tstring& name, const DWORD timeout_ms)
{
    InfoLog(TSTR("StartupService[%s] begin"), name.c_str());
    bool bReturn = false;

    do 
    {
        scoped_svc_handle hService(name, GENERIC_ALL, GENERIC_EXECUTE | GENERIC_READ);
        if (!hService.valid())
        {
            break;
        }

        SERVICE_STATUS status = {0};
        if (!QueryServiceStatus(hService.get(), &status))
        {
            ErrorLogLastErr(CLastErrorFormat(), TSTR("QueryServiceStatus[%s] fail"), name.c_str());
            break;
        }

        if (SERVICE_RUNNING == status.dwCurrentState)
        {
            bReturn = true;
            break;
        }

        if (SERVICE_START_PENDING != status.dwCurrentState)
        {
            if (!StartService(hService.get(), 0, NULL))
            {
                CLastErrorFormat e;
                if (ERROR_SERVICE_ALREADY_RUNNING == e.code())
                {
                    DebugLog("service is already running");
                    bReturn = true;
                }
                else
                {
                    ErrorLogLastErr(e, TSTR("StartService[%s] fail"), name.c_str());
                }
                break;
            }
        }

        const DWORD interval_ms = 500;//每500毫秒检测一次
        DWORD total_ms = timeout_ms > interval_ms ? timeout_ms : interval_ms;
        DWORD total_count = total_ms / interval_ms;
        for (DWORD index = 0; index != total_count; ++index)
        {
            if (!QueryServiceStatus(hService.get(), &status))
            {
                ErrorLogLastErr(CLastErrorFormat(), TSTR("QueryServiceStatus[%s] fail"), name.c_str());
                break;
            }

            if (SERVICE_START_PENDING == status.dwCurrentState)
            {
                DebugLog("SERVICE_START_PENDING");
                Sleep(interval_ms);
            }
            else
            {
                break;
            }
        }

        if (SERVICE_RUNNING != status.dwCurrentState)
        {
            ErrorLog(TSTR("start service[%s] fail, current status: %lu, %s"),
                name.c_str(), status.dwCurrentState, service_status_str(status.dwCurrentState));
            break;
        }

        bReturn = true;

    } while (false);

    InfoLog(TSTR("StartupService[%s] end"), name.c_str());
    return bReturn;
}

bool ServiceUtil::stop(const tstring& name, const DWORD timeout_ms)
{
    InfoLog(TSTR("StopService[%s] begin"), name.c_str());
    bool bReturn = false;

    do 
    {
        scoped_svc_handle hService(name, GENERIC_ALL, GENERIC_EXECUTE | GENERIC_READ);
        if (!hService.valid())
        {
            break;
        }

        SERVICE_STATUS status = {0};
        if (!QueryServiceStatus(hService.get(), &status))
        {
            ErrorLogLastErr(CLastErrorFormat(), TSTR("QueryServiceStatus[%s] fail"), name.c_str());
            break;
        }

        if (SERVICE_STOPPED == status.dwCurrentState)
        {
            bReturn = true;
            break;
        }

        if (SERVICE_STOP_PENDING != status.dwCurrentState)
        {
            if (!ControlService(hService.get(), SERVICE_CONTROL_STOP, &status))
            {
                ErrorLogLastErr(CLastErrorFormat(), TSTR("ControlService[%s] for stopping fail"), name.c_str());
                break;
            }
        }

        const DWORD interval_ms = 500;//每500毫秒检测一次
        DWORD total_ms = timeout_ms > interval_ms ? timeout_ms : interval_ms;
        DWORD total_count = total_ms / interval_ms;
        for (DWORD index = 0; index != total_count; ++index)
        {
            if (!QueryServiceStatus(hService.get(), &status))
            {
                ErrorLogLastErr(CLastErrorFormat(), TSTR("QueryServiceStatus[%s] fail"), name.c_str());
                break;
            }

            if (SERVICE_RUNNING == status.dwCurrentState)
            {
                DebugLog("SERVICE_RUNNING");
                Sleep(interval_ms);
            }
            else if (SERVICE_STOP_PENDING == status.dwCurrentState)
            {
                DebugLog("SERVICE_STOP_PENDING");
                Sleep(interval_ms);
            }
            else
            {
                break;
            }
        }

        if (SERVICE_STOPPED != status.dwCurrentState)
        {
            ErrorLog(TSTR("stop service[%s] fail, current status: %lu, %s"),
                name.c_str(), status.dwCurrentState, service_status_str(status.dwCurrentState));
            break;
        }

        bReturn = true;

    } while (false);

    InfoLog(TSTR("StopService[%s] end"), name.c_str());
    return bReturn;
}

bool ServiceUtil::send_control_code(const tstring& name, const DWORD code)
{
    bool bReturn = false;

    do 
    {
        scoped_svc_handle hService(name, GENERIC_WRITE | GENERIC_EXECUTE, GENERIC_WRITE | GENERIC_EXECUTE);
        if (!hService.valid())
        {
            break;
        }

        SERVICE_STATUS status = {0};
        if (!ControlService(hService.get(), code, &status))
        {
            ErrorLogLastErr(CLastErrorFormat(), TSTR("ControlService[%s:%lu] fail"), name.c_str(), code);
            break;
        }

        bReturn = true;

    } while (false);

    return bReturn;
}


