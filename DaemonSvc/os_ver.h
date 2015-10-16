#pragma once


struct OS_VER
{
    enum VER
    {
        VER_UNKNOWN = 0,
        WIN_2K,
        WIN_XP,
        WIN_SERVER_2003,
        WIN_HOME_SERVER,
        WIN_SERVER_2003_R2,
        WIN_VISTA,
        WIN_SERVER_2008,
        WIN_SERVER_2008_R2,
        WIN_7,
        WIN_SERVER_2012,
        WIN_8,
        WIN_SERVER_2012_R2,
        WIN_8_1,

    };


    VER v;
    unsigned long major_version;
    unsigned long minor_version;
    unsigned short sp_major_version;
    unsigned short sp_minor_version;

    OS_VER()
        : v(VER_UNKNOWN)
        , major_version(0)
        , minor_version(0)
        , sp_major_version(0)
        , sp_minor_version(0)
    {
    }
};



bool is_64bits_os();
OS_VER get_os_version();
