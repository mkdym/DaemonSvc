#pragma once
#include <Windows.h>
#include "common.h"
#include "service_info.h"


namespace ServiceUtil
{
    bool IsServiceExist(const tstring& name);
    bool IsServiceRunning(const tstring& name);

    bool InstallService(const ServiceInfo& info, const tstring& binary_file);
    bool RemoveService(const tstring& name);

    bool StartupService(const tstring& name, const DWORD timeout_ms);
    bool StopService(const tstring& name, const DWORD timeout_ms);

    bool SendControlCode2Service(const tstring& name, const DWORD code);
}
