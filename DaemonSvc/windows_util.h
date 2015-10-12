//this file contains some encapsulated functions
//they are all related to windows operating system functions
//and I don't know how to classify these, so just put them here
#pragma once
#include <string>
#include <Windows.h> // for FARPROC
#include "tdef.h"


namespace WindowsUtil
{
    //do not need to consider std::wstring
    //because module_name and func_name are always hard-coded
    FARPROC load_function(const std::string& module_name,
        const std::string& func_name,
        const bool log = true);
    //use tstring, because Windows privilege macros are TEXT("xxx") in winnt.h
    bool set_privilege(const tstring& privilege_name, const bool enable);
}


