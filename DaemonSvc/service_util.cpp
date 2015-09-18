#pragma once
#include <Windows.h>
#include <boost/algorithm/string.hpp>
#include <boost/smart_ptr.hpp>
#include "logger.h"
#include "service_util.h"


#pragma comment(lib, "Advapi32.lib")



bool ServiceUtil::IsServiceExist(const tstring& name)
{
    bool bReturn = false;

    SC_HANDLE hSCMgr = NULL;
    do 
    {
        hSCMgr = OpenSCManager(NULL, NULL, GENERIC_READ);
        if (NULL == hSCMgr)
        {
            ErrorLogA("OpenSCManager fail, error code: %d", GetLastError());
            break;
        }

        DWORD dwNeededBytes = 0;
        DWORD dwReturnedSerivice = 0;
        DWORD dwResumeEntryNum = 0;
        if (EnumServicesStatusEx(hSCMgr, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER | SERVICE_WIN32, 
            SERVICE_STATE_ALL, NULL, 0, &dwNeededBytes, &dwReturnedSerivice, &dwResumeEntryNum, NULL))
        {
            ErrorLogA("EnumServicesStatusEx success while query needed bytes");
            break;
        }

        if (ERROR_MORE_DATA != GetLastError())
        {
            ErrorLogA("EnumServicesStatusEx error code is not ERROR_MORE_DATA while query needed bytes");
            break;
        }

        boost::scoped_array<BYTE> lpData(new BYTE[dwNeededBytes]);
        memset(lpData.get(), 0, dwNeededBytes);
        if (!EnumServicesStatusEx(hSCMgr, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER | SERVICE_WIN32, 
            SERVICE_STATE_ALL, lpData.get(), dwNeededBytes, &dwNeededBytes, &dwReturnedSerivice, &dwResumeEntryNum, NULL))
        {
            ErrorLogA("EnumServicesStatusEx fail, error code: %d", GetLastError());
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

    if (hSCMgr)
    {
        CloseServiceHandle(hSCMgr);
        hSCMgr = NULL;
    }

    return bReturn;
}

bool ServiceUtil::IsServiceRunning(const tstring& name)
{
    bool bReturn = false;

    SC_HANDLE hSCMgr = NULL;
    SC_HANDLE hService = NULL;
    do 
    {
        hSCMgr = OpenSCManager(NULL, NULL, GENERIC_READ);
        if (NULL == hSCMgr)
        {
            ErrorLogA("OpenSCManager fail, error code: %d", GetLastError());
            break;
        }

        hService = OpenService(hSCMgr, name.c_str(), GENERIC_READ);
        if (NULL == hService)
        {
            ErrorLog(TEXT("OpenService[%s] fail, error code: %d"), name.c_str(), GetLastError());
            break;
        }

        SERVICE_STATUS status = {0};
        if (!QueryServiceStatus(hService, &status))
        {
            ErrorLog(TEXT("QueryServiceStatus[%s] fail, error code: %d"), name.c_str(), GetLastError());
            break;
        }

        if (SERVICE_RUNNING != status.dwCurrentState)
        {
            DebugLog(TEXT("service[%s] is not running. status: %d"), name.c_str(), status.dwCurrentState);
            break;
        }

        bReturn = true;

    } while (false);

    if (hSCMgr)
    {
        CloseServiceHandle(hSCMgr);
        hSCMgr = NULL;
    }
    if (hService)
    {
        CloseServiceHandle(hService);
        hService = NULL;
    }

    return bReturn;
}

bool ServiceUtil::InstallService(const ServiceInfo& info, const tstring& binary_file)
{
    bool bReturn = false;

    SC_HANDLE hSCMgr = NULL;
    SC_HANDLE hService = NULL;
    do 
    {
        hSCMgr = OpenSCManager(NULL, NULL, GENERIC_ALL);
        if (NULL == hSCMgr)
        {
            ErrorLogA("OpenSCManager fail, error code: %d", GetLastError());
            break;
        }

        DWORD tag_id = info.tag_id;
        hService = CreateService(hSCMgr,
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
            ErrorLog(TEXT("CreateService[%s] fail, error code: %d"), info.name.c_str(), GetLastError());
            break;
        }

        bReturn = true;

    } while (false);

    if (hSCMgr)
    {
        CloseServiceHandle(hSCMgr);
        hSCMgr = NULL;
    }
    if (hService)
    {
        CloseServiceHandle(hService);
        hService = NULL;
    }

    return bReturn;
}

bool ServiceUtil::RemoveService(const tstring& name)
{
    bool bReturn = false;

    SC_HANDLE hSCMgr = NULL;
    SC_HANDLE hService = NULL;
    do 
    {
        hSCMgr = OpenSCManager(NULL, NULL, GENERIC_ALL);
        if (NULL == hSCMgr)
        {
            ErrorLogA("OpenSCManager fail, error code: %d", GetLastError());
            break;
        }

        hService = OpenService(hSCMgr, name.c_str(), SERVICE_ALL_ACCESS);
        if (NULL == hService)
        {
            ErrorLog(TEXT("OpenService[%s] fail, error code: %d"), name.c_str(), GetLastError());
            break;
        }

        if (!DeleteService(hService))
        {
            ErrorLog(TEXT("DeleteService[%s] fail, error code: %d"), name.c_str(), GetLastError());
            break;
        }

        bReturn = true;

    } while (false);

    if (hSCMgr)
    {
        CloseServiceHandle(hSCMgr);
        hSCMgr = NULL;
    }
    if (hService)
    {
        CloseServiceHandle(hService);
        hService = NULL;
    }

    return bReturn;
}

bool ServiceUtil::StartupService(const tstring& name, const DWORD timeout_ms)
{
    DebugLog(TEXT("StartupService[%s] begin"), name.c_str());
    bool bReturn = false;

    SC_HANDLE hSCMgr = NULL;
    SC_HANDLE hService = NULL;
    do 
    {
        hSCMgr = OpenSCManager(NULL, NULL, GENERIC_ALL);
        if (NULL == hSCMgr)
        {
            ErrorLogA("OpenSCManager fail, error code: %d", GetLastError());
            break;
        }

        hService = OpenService(hSCMgr, name.c_str(), GENERIC_EXECUTE | GENERIC_READ);
        if (NULL == hService)
        {
            ErrorLog(TEXT("OpenService[%s] fail, error code: %d"), name.c_str(), GetLastError());
            break;
        }

        SERVICE_STATUS status = {0};
        if (!QueryServiceStatus(hService, &status))
        {
            ErrorLog(TEXT("QueryServiceStatus[%s] fail, error code: %d"), name.c_str(), GetLastError());
            break;
        }

        if (SERVICE_RUNNING == status.dwCurrentState)
        {
            bReturn = true;
            break;
        }

        if (SERVICE_START_PENDING != status.dwCurrentState)
        {
            if (!StartService(hService, 0, NULL))
            {
                const DWORD e = GetLastError();
                if (ERROR_SERVICE_ALREADY_RUNNING == e)
                {
                    DebugLogA("service is already running");
                    bReturn = true;
                }
                else
                {
                    ErrorLog(TEXT("StartService[%s] fail, error code: %d"), name.c_str(), e);
                }
                break;
            }
        }

        const DWORD interval_ms = 500;//每500毫秒检测一次
        DWORD total_ms = timeout_ms > interval_ms ? timeout_ms : interval_ms;
        DWORD total_count = total_ms / interval_ms;
        for (DWORD index = 0; index != total_count; ++index)
        {
            if (!QueryServiceStatus(hService, &status))
            {
                ErrorLog(TEXT("QueryServiceStatus[%s] fail, error code: %d"), name.c_str(), GetLastError());
                break;
            }

            if (SERVICE_START_PENDING == status.dwCurrentState)
            {
                DebugLogA("SERVICE_START_PENDING");
                Sleep(interval_ms);
            }
            else
            {
                break;
            }
        }

        if (SERVICE_RUNNING != status.dwCurrentState)
        {
            ErrorLog(TEXT("start service[%s] fail, current status: %d"), name.c_str(), status.dwCurrentState);
            break;
        }

        bReturn = true;

    } while (false);

    if (hSCMgr)
    {
        CloseServiceHandle(hSCMgr);
        hSCMgr = NULL;
    }
    if (hService)
    {
        CloseServiceHandle(hService);
        hService = NULL;
    }

    DebugLog(TEXT("StartupService[%s] end"), name.c_str());
    return bReturn;
}

bool ServiceUtil::StopService(const tstring& name, const DWORD timeout_ms)
{
    DebugLog(TEXT("StopService[%s] begin"), name.c_str());
    bool bReturn = false;

    SC_HANDLE hSCMgr = NULL;
    SC_HANDLE hService = NULL;
    do 
    {
        hSCMgr = OpenSCManager(NULL, NULL, GENERIC_ALL);
        if (NULL == hSCMgr)
        {
            ErrorLogA("OpenSCManager fail, error code: %d", GetLastError());
            break;
        }

        hService = OpenService(hSCMgr, name.c_str(), GENERIC_EXECUTE | GENERIC_READ);
        if (NULL == hService)
        {
            ErrorLog(TEXT("OpenService[%s] fail, error code: %d"), name.c_str(), GetLastError());
            break;
        }

        SERVICE_STATUS status = {0};
        if (!QueryServiceStatus(hService, &status))
        {
            ErrorLog(TEXT("QueryServiceStatus[%s] fail, error code: %d"), name.c_str(), GetLastError());
            break;
        }

        if (SERVICE_STOPPED == status.dwCurrentState)
        {
            bReturn = true;
            break;
        }

        if (SERVICE_STOP_PENDING != status.dwCurrentState)
        {
            if (!ControlService(hService, SERVICE_CONTROL_STOP, &status))
            {
                ErrorLog(TEXT("ControlService[%s] for stopping fail, error code: %d"), name.c_str(), GetLastError());
                break;
            }
        }

        const DWORD interval_ms = 500;//每500毫秒检测一次
        DWORD total_ms = timeout_ms > interval_ms ? timeout_ms : interval_ms;
        DWORD total_count = total_ms / interval_ms;
        for (DWORD index = 0; index != total_count; ++index)
        {
            if (!QueryServiceStatus(hService, &status))
            {
                ErrorLog(TEXT("QueryServiceStatus[%s] fail, error code: %d"), name.c_str(), GetLastError());
                break;
            }

            if (SERVICE_RUNNING == status.dwCurrentState)
            {
                DebugLogA("SERVICE_RUNNING");
                Sleep(interval_ms);
            }
            else if (SERVICE_STOP_PENDING == status.dwCurrentState)
            {
                DebugLogA("SERVICE_STOP_PENDING");
                Sleep(interval_ms);
            }
            else
            {
                break;
            }
        }

        if (SERVICE_STOPPED != status.dwCurrentState)
        {
            ErrorLog(TEXT("stop service[%s] fail, current status: %d"), name.c_str(), status.dwCurrentState);
            break;
        }

        bReturn = true;

    } while (false);

    if (hSCMgr)
    {
        CloseServiceHandle(hSCMgr);
        hSCMgr = NULL;
    }
    if (hService)
    {
        CloseServiceHandle(hService);
        hService = NULL;
    }

    DebugLog(TEXT("StopService[%s] end"), name.c_str());
    return bReturn;
}

bool ServiceUtil::SendControlCode2Service(const tstring& name, const DWORD code)
{
    bool bReturn = false;

    SC_HANDLE hSCMgr = NULL;
    SC_HANDLE hService = NULL;
    do 
    {
        hSCMgr = OpenSCManager(NULL, NULL, GENERIC_WRITE | GENERIC_EXECUTE);
        if (NULL == hSCMgr)
        {
            ErrorLogA("OpenSCManager fail, error code: %d", GetLastError());
            break;
        }

        hService = OpenService(hSCMgr, name.c_str(), GENERIC_WRITE | GENERIC_EXECUTE);
        if (NULL == hService)
        {
            ErrorLog(TEXT("OpenService[%s] fail, error code: %d"), name.c_str(), GetLastError());
            break;
        }

        SERVICE_STATUS status = {0};
        if (!ControlService(hService, code, &status))
        {
            ErrorLog(TEXT("ControlService[%s:%d] fail, error code: %d"), name.c_str(), code, GetLastError());
            break;
        }

        bReturn = true;

    } while (false);

    if (hSCMgr)
    {
        CloseServiceHandle(hSCMgr);
        hSCMgr = NULL;
    }
    if (hService)
    {
        CloseServiceHandle(hService);
        hService = NULL;
    }

    return bReturn;
}


