#pragma once
#include <Windows.h>
#include <boost/noncopyable.hpp>
#include "tdef.h"


class CDaemon : public boost::noncopyable
{
private:
    CDaemon(void);
    ~CDaemon(void);

public:
    static CDaemon& GetInstanceRef()
    {
        static CDaemon instance;
        return instance;
    }

public:
    bool start();
    void keep_running();
    void notify_stop();
    void restart();

private:
    bool start_tasks_by_config(const tstring& config_file);

private:
    HANDLE m_exit_event;
};
