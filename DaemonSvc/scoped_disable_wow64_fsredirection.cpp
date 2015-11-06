#include <Windows.h>
#include <boost/thread/once.hpp>
#include "logger.h"
#include "windows_util.h"
#include "scoped_disable_wow64_fsredirection.h"



static boost::once_flag once_ = BOOST_ONCE_INIT;


typedef int (__stdcall *fnWow64DisableWow64FsRedirection)(void *);
typedef int (__stdcall *fnWow64RevertWow64FsRedirection)(void *);

static fnWow64DisableWow64FsRedirection g_fnWow64DisableWow64FsRedirection = NULL;
static fnWow64RevertWow64FsRedirection g_fnWow64RevertWow64FsRedirection = NULL;


static void load_wow64_funcs()
{
    g_fnWow64DisableWow64FsRedirection = reinterpret_cast<fnWow64DisableWow64FsRedirection>
        (WindowsUtil::load_function("Kernel32.dll", "Wow64DisableWow64FsRedirection"));

    g_fnWow64RevertWow64FsRedirection = reinterpret_cast<fnWow64RevertWow64FsRedirection>
        (WindowsUtil::load_function("Kernel32.dll", "Wow64RevertWow64FsRedirection"));
}



scoped_disable_wow64_fsredirection::scoped_disable_wow64_fsredirection()
    : _pOldValue(NULL)
{
    boost::call_once(once_, load_wow64_funcs);
    disable(&_pOldValue);
}

scoped_disable_wow64_fsredirection::~scoped_disable_wow64_fsredirection()
{
    revert(_pOldValue);
}


bool scoped_disable_wow64_fsredirection::disable(void **ppOldValue)
{
    bool ret = true;

    if (g_fnWow64DisableWow64FsRedirection)
    {
        if (!g_fnWow64DisableWow64FsRedirection(ppOldValue))
        {
            ErrorLogLastErr("Wow64DisableWow64FsRedirection fail");
            ret = false;
        }
    }

    return ret;
}

bool scoped_disable_wow64_fsredirection::revert(void *pOldValue)
{
    bool ret = true;

    if (g_fnWow64RevertWow64FsRedirection)
    {
        if (!g_fnWow64RevertWow64FsRedirection(pOldValue))
        {
            ErrorLogLastErr("Wow64RevertWow64FsRedirection fail");
            ret = false;
        }
    }

    return ret;
}





