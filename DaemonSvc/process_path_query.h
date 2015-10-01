#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "tdef.h"


//you should enable debug privilege if you want query other user's process
//typically, you can call WindowsUtil::set_privilege(SE_DEBUG_NAME, true)
class CProcessPathQuery : public boost::noncopyable
{
public:
    CProcessPathQuery(void);
    ~CProcessPathQuery(void);

public:
    //return empty string if error
    //on return, if native_name is true, returned string is native name
    //for example, \Device\Harddisk0\Partition1\WINNT\System32\Ctype.nls
    //see GetProcessImageFileNameW in MSDN
    //you can use xxx to convert it to DOS name: C:\Winnt\System32\Ctype.nls
    //I do not put the conversion in "query", because it costs some time to prepare
    //once prepared, you can use it for many conversions
    //typical scenario is in process scanner
    //found in testing:
    //      on Windows 2000, you will get
    //              "\SystemRoot\System32\smss.exe" for smss.exe
    //              "\??\C:\WINNT\system32\csrss.exe" for csrss.exe
    //              "\??\C:\WINNT\system32\winlogon.exe" for winlogon.exe
    //      on all Windows, can not get "System Process(System Idle Process)", "System" process path
    //              however, it is no sense to get their process path
    //              on Windows 2000, "System Process" id is 0, "System" is 4
    //              on other Windows, "System Process" id is 0, "System" is 8
    //      on Windows Vista/2008/7/2008R2, can not get process path of "audiodg.exe"
    //              it may be protected by kernel Driver
    //              however, it is also no sense to get its process path
    //              but you will get an error log
    tstring query(const DWORD pid, bool& native_name);

    //the process handle must have PROCESS_QUERY_INFORMATION | PROCESS_VM_READ privileges
    tstring query(HANDLE hProcess, bool& native_name);
};
