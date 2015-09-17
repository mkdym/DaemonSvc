#pragma once
#include <Windows.h>
#include <boost\algorithm\string.hpp>
#include "logger.h"
#include "ServiceUtil.h"


#pragma comment(lib, "Advapi32.lib")



bool ServiceUtil::IsServiceExist(const tstring& name)
{
    return false;
}

bool ServiceUtil::IsServiceRunning(const tstring& name)
{
    return false;
}

bool ServiceUtil::InstallService(const ServiceInfo& info)
{
    return false;
}

bool ServiceUtil::RemoveService(const tstring& name)
{
    return false;
}

bool ServiceUtil::StartupService(const tstring& name, const DWORD timeout_ms)
{
    return false;
}

bool ServiceUtil::StopService(const tstring& name, const DWORD timeout_ms)
{
    return false;
}

bool ServiceUtil::SendControlCode2Service(const tstring& name, const DWORD code)
{
    return false;
}


