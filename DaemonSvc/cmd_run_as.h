#pragma once
#include <string>
#include "tdef.h"


enum RUN_AS_TYPE
{
    AS_UNKNOWN = 0,             //error
    AS_LOCAL,                   //run in local context
    AS_LOGON_USER,              //run as one logon user
    AS_ALL_LOGON_USERS,         //run as all logon users
};


RUN_AS_TYPE cast_run_as_type_from_string(const std::string& s);
std::string cast_run_as_type_to_string(const RUN_AS_TYPE& run_as);


//************************************
// brief:    execute command
// name:     run_as
// param:    const tstring & command            command line
// param:    const RUN_AS_TYPE & as_type
// param:    const bool show_window             SW_SHOWNORMAL if true, otherwise SW_HIDE. see ShowWindow in MSDN
// return:   bool                               return true if has created any process successfully
// remarks:  if AS_LOGON_USER, exec in all logon users context(one-by-one) until one is successful
//           if AS_ALL_LOGON_USERS, exec in all logon users context whether or not successful
//************************************
bool cmd_run_as(const tstring& command,
                const RUN_AS_TYPE& as_type,
                const bool show_window = true);

