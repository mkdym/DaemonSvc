#include <Windows.h>
#include <boost/thread/once.hpp>
#include "logger.h"
#include "windows_util.h"
#include "os_ver.h"



#ifndef VER_SUITE_WH_SERVER
#define VER_SUITE_WH_SERVER     (0x00008000)
#endif

#ifndef SM_SERVERR2
#define SM_SERVERR2             89
#endif



static bool g_b64bits_os = false;
static OS_VER g_os_ver;



static boost::once_flag once_for_query_64bits;
static void query_64bits()
{
    typedef UINT (WINAPI *fnGetSystemWow64DirectoryA)(LPSTR, UINT);
    fnGetSystemWow64DirectoryA _GetSystemWow64DirectoryA = reinterpret_cast<fnGetSystemWow64DirectoryA>
        (WindowsUtil::load_function("Kernel32.dll", "GetSystemWow64DirectoryA"));

    if (NULL == _GetSystemWow64DirectoryA)
    {
        g_b64bits_os = false;
    }
    else
    {
        char buf[1] = {0};
        if (0 == _GetSystemWow64DirectoryA(buf, 1) && ERROR_CALL_NOT_IMPLEMENTED == GetLastError())
        {
            g_b64bits_os = false;
        }
        else
        {
            g_b64bits_os = true;
        }
    }
}

bool is_64bits_os()
{
    boost::call_once(once_for_query_64bits, query_64bits);
    return g_b64bits_os;
}


static boost::once_flag once_for_query_os_ver;
static void query_os_version()
{
    g_os_ver.v = OS_VER::VER_UNKNOWN;

    OSVERSIONINFOEX osvi = {0};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (!GetVersionEx(reinterpret_cast<OSVERSIONINFO *>(&osvi)))
    {
        ErrorLogLastErr("GetVersionEx fail");
    }
    else
    {
        switch (osvi.dwMajorVersion)
        {
        case 5:
            switch (osvi.dwMinorVersion)
            {
            case 0:
                g_os_ver.v = OS_VER::WIN_2K;
                break;

            case 1:
                g_os_ver.v = OS_VER::WIN_XP;
                break;

            case 2:
                if (VER_SUITE_WH_SERVER & osvi.wSuiteMask)
                {
                    g_os_ver.v = OS_VER::WIN_HOME_SERVER;
                }
                else
                {
                    if (0 == GetSystemMetrics(SM_SERVERR2))
                    {
                        g_os_ver.v = OS_VER::WIN_SERVER_2003;
                    }
                    else
                    {
                        if (osvi.wProductType == VER_NT_WORKSTATION && is_64bits_os())
                        {
                            g_os_ver.v = OS_VER::WIN_XP;
                        }
                        else
                        {
                            g_os_ver.v = OS_VER::WIN_SERVER_2003_R2;
                        }
                    }
                }
                break;

            default:
                break;
            }
            break;

        case 6:
            switch (osvi.dwMinorVersion)
            {
            case 0:
                if (VER_NT_WORKSTATION == osvi.wProductType)
                {
                    g_os_ver.v = OS_VER::WIN_VISTA;
                }
                else
                {
                    g_os_ver.v = OS_VER::WIN_SERVER_2008;
                }
                break;

            case 1:
                if (VER_NT_WORKSTATION == osvi.wProductType)
                {
                    g_os_ver.v = OS_VER::WIN_7;
                }
                else
                {
                    g_os_ver.v = OS_VER::WIN_SERVER_2008_R2;
                }
                break;

            case 2:
                if (VER_NT_WORKSTATION == osvi.wProductType)
                {
                    g_os_ver.v = OS_VER::WIN_8;
                }
                else
                {
                    g_os_ver.v = OS_VER::WIN_SERVER_2012;
                }
                break;

            case 3:
                if (VER_NT_WORKSTATION == osvi.wProductType)
                {
                    g_os_ver.v = OS_VER::WIN_8_1;
                }
                else
                {
                    g_os_ver.v = OS_VER::WIN_SERVER_2012_R2;
                }
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }

        if (OS_VER::VER_UNKNOWN == g_os_ver.v)
        {
            ErrorLog("system is too old or new to tell the version exactly: MajorVersion=%lu, MinorVersion=%lu, ServicePackMajor=%d, ServicePackMinor=%d, SuiteMask=0x%x, ProductType=0x%x",
                osvi.dwMajorVersion, osvi.dwMinorVersion,
                osvi.wServicePackMajor, osvi.wServicePackMinor,
                osvi.wSuiteMask, osvi.wProductType);
        }

        g_os_ver.major_version = osvi.dwMajorVersion;
        g_os_ver.minor_version = osvi.dwMinorVersion;
        g_os_ver.sp_major_version = osvi.wServicePackMajor;
        g_os_ver.sp_minor_version = osvi.wServicePackMinor;
    }
}

OS_VER get_os_version()
{
    boost::call_once(once_for_query_os_ver, query_os_version);
    return g_os_ver;
}


