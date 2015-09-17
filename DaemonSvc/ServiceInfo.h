#pragma once
#include <Windows.h>
#include "common.h"



struct ServiceInfo
{
    tstring name;
    tstring display_name;
    DWORD desired_access;
    DWORD service_type;
    DWORD start_type;
    DWORD error_control;
    tstring load_order_group;

    bool use_tagid;
    DWORD tag_id;

    tstring dependencies;

    bool use_startname;
    tstring start_name;

    bool use_password;
    tstring password;

    DWORD accepted_controls;

    ServiceInfo()
        : desired_access(SERVICE_ALL_ACCESS)
        , service_type(SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS)
        , start_type(SERVICE_AUTO_START)
        , error_control(SERVICE_ERROR_NORMAL)
        , use_tagid(false)
        , use_startname(false)
        , use_password(false)
        , accepted_controls(SERVICE_ACCEPT_STOP)
    {
    }
};

