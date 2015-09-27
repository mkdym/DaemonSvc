//this file contains some encapsulated functions
//they are all related to windows operating system functions
//and I don't know how to classify these, so just put them here
#pragma once
#include "tdef.h"


namespace WindowsUtil
{
    FARPROC load_function(const tstring& module_name,
        const tstring& func_name,
        const bool log = true);
    bool set_privilege(const tstring& privilege_name, const bool enable);
}


