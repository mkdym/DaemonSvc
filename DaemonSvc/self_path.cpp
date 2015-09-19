#pragma once
#include <Windows.h>
#include "last_error.h"
#include "self_path.h"



bool CSelfPath::init()
{
    if (m_has_init)
    {
        return true;
    }
    else
    {
        std::cout << "init self path" << std::endl;

        //size limit
        const DWORD full_buf_size = 2048;
        tchar full_path_buf[full_buf_size] = {0};
        if (!GetModuleFileName(NULL, full_path_buf, full_buf_size - 1))
        {
            print_last_err(CLastError(), TEXT("GetModuleBaseName fail"));
            return false;
        }
        else
        {
            m_full = full_path_buf;

            const DWORD driver_buf_size = 4;
            const DWORD ext_buf_size = 260;
            const DWORD name_buf_size = 260;
            const DWORD dir_buf_size = full_buf_size - name_buf_size - ext_buf_size - driver_buf_size;

            tchar driver_buf[driver_buf_size] = {0};
            tchar ext_buf[ext_buf_size] = {0};
            tchar name_buf[name_buf_size] = {0};
            tchar dir_buf[dir_buf_size] = {0};

            _tsplitpath_s(full_path_buf,
                driver_buf, driver_buf_size,
                dir_buf, dir_buf_size,
                name_buf, name_buf_size,
                ext_buf, ext_buf_size);

            driver_buf[driver_buf_size - 1] = TEXT('\0');
            dir_buf[dir_buf_size - 1] = TEXT('\0');
            name_buf[name_buf_size - 1] = TEXT('\0');
            ext_buf[ext_buf_size - 1] = TEXT('\0');

            m_dir = driver_buf;
            m_dir += dir_buf;
            m_name = name_buf;
            m_ext = ext_buf;

            m_has_init = true;
            return true;
        }
    }
}
