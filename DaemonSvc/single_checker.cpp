#pragma once
#include "logger.h"
#include "single_checker.h"


CSingleChecker::CSingleChecker(void)
{
}

CSingleChecker::~CSingleChecker(void)
{
}

bool CSingleChecker::single(const tstring& mutex_name)
{
    if (m_mutex.valid())//we created the mutex before
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
                ErrorLogLastErr("InitializeSecurityDescriptor fail");
                break;
            }
            if (!SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE))
            {
                ErrorLogLastErr("SetSecurityDescriptorDacl fail");
                break;
            }

            SECURITY_ATTRIBUTES sa;
            sa.nLength =sizeof(SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = &sd;
            sa.bInheritHandle = TRUE;

            tstring mutex_global_name(TSTR("Global\\"));
            mutex_global_name += mutex_name;
            m_mutex.reset(CreateMutex(&sa, FALSE, mutex_global_name.c_str()));

            CLastErrorFormat e;
            if (m_mutex.valid())
            {
                //创建成功且错误不是“已存在”，即确确实实是自己创建的，那么认为只有一个实例在运行，保留此句柄
                if (e.code() != ERROR_ALREADY_EXISTS)
                {
                    bSingle = true;
                }
                else
                {
                    m_mutex.destory();
                }
            }
            else
            {
                //创建失败且错误是“权限不足”，即已被别人创建，自己不能打开，则可确认有另外的实例已在运行
                //if (e.code() == ERROR_ACCESS_DENIED)
                //{
                //}
                ErrorLogLastErrEx(e, TSTR("CreateMutex[%s] fail"), mutex_global_name.c_str());
            }

        } while (false);
        return bSingle;
    }
}



