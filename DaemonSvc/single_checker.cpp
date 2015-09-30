#pragma once
#include "logger.h"
#include "single_checker.h"


CSingleChecker::CSingleChecker(void)
    : m_mutex(NULL)
{
}

CSingleChecker::~CSingleChecker(void)
{
    if (m_mutex)
    {
        CloseHandle(m_mutex);
        m_mutex = NULL;
    }
}

bool CSingleChecker::single(const tstring& mutex_name)
{
    if (m_mutex)//we created the mutex before
    {
        return true;
    }
    else
    {
        bool bSingle = false;
        do 
        {
            SECURITY_DESCRIPTOR sd;
            if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
            {
                ErrorLogLastErr(CLastError(), "InitializeSecurityDescriptor fail");
                break;
            }
            if (!SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE))
            {
                ErrorLogLastErr(CLastError(), "SetSecurityDescriptorDacl fail");
                break;
            }

            SECURITY_ATTRIBUTES sa;
            sa.nLength =sizeof(SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = &sd;
            sa.bInheritHandle = TRUE;

            tstring mutex_global_name(TSTR("Global\\"));
            mutex_global_name += mutex_name;
            m_mutex = CreateMutex(&sa, FALSE, mutex_global_name.c_str());

            CLastError e;
            if (m_mutex)
            {
                //创建成功且错误不是“已存在”，即确确实实是自己创建的，那么认为只有一个实例在运行，保留此句柄
                if (e.code() != ERROR_ALREADY_EXISTS)
                {
                    bSingle = true;
                }
                else
                {
                    CloseHandle(m_mutex);
                    m_mutex = NULL;
                }
            }
            else
            {
                //创建失败且错误是“权限不足”，即已被别人创建，自己不能打开，则可确认有另外的实例已在运行
                //if (e.code() == ERROR_ACCESS_DENIED)
                //{
                //}
                ErrorLogLastErr(e, TSTR("CreateMutex[%s] fail"), mutex_global_name.c_str());
            }

        } while (false);
        return bSingle;
    }
}



