#pragma once
#include "tdef.h"
#include "service_info.h"


namespace ServiceUtil
{
    bool is_exist(const tstring& name);
    bool is_running(const tstring& name);

    bool install(const ServiceInfo& info, const tstring& binary_file);
    bool remove(const tstring& name);

    bool startup(const tstring& name, const DWORD timeout_ms);
    bool stop(const tstring& name, const DWORD timeout_ms);

    bool send_control_code(const tstring& name, const DWORD code);
}
