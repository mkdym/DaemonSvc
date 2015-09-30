#pragma once
#include <Windows.h>
#include "last_error_format.h"
#include "self_path.h"


CSelfPath::CSelfPath(void)
{
    //todo: size limit
    const DWORD full_buf_size = 2048;
    char full_path_buf[full_buf_size] = {0};

    if (!GetModuleFileNameA(NULL, full_path_buf, full_buf_size - 1))
    {
        print_last_err(CLastErrorFormat(), "GetModuleBaseName fail");
    }
    else
    {
        m_full = full_path_buf;//we have reserved one char to fix the buf to be null-terminated

        const DWORD driver_buf_size = 4;
        const DWORD ext_buf_size = 260;
        const DWORD name_buf_size = 260;
        const DWORD dir_buf_size = full_buf_size - name_buf_size - ext_buf_size - driver_buf_size;

        char driver_buf[driver_buf_size] = {0};
        char ext_buf[ext_buf_size] = {0};
        char name_buf[name_buf_size] = {0};
        char dir_buf[dir_buf_size] = {0};

        _splitpath_s(full_path_buf,
            driver_buf, driver_buf_size,
            dir_buf, dir_buf_size,
            name_buf, name_buf_size,
            ext_buf, ext_buf_size);

        driver_buf[driver_buf_size - 1] = 0;
        dir_buf[dir_buf_size - 1] = 0;
        name_buf[name_buf_size - 1] = 0;
        ext_buf[ext_buf_size - 1] = 0;

        m_dir = driver_buf;
        m_dir += dir_buf;
        m_name = name_buf;
        m_ext = ext_buf;
    }
}


