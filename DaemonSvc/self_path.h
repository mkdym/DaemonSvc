#pragma once
#include <boost/noncopyable.hpp>
#include "common.h"


//do not use log, because we use this class to init log module
class CSelfPath : boost::noncopyable
{
private:
    CSelfPath(void)
        : m_has_init(false)
    {
    }
    ~CSelfPath(void)
    {
    }

public:
    static CSelfPath& GetInstanceRef()
    {
        static CSelfPath instance;
        return instance;
    }

public:
    bool init();

public:
    const tstring& get_full() const
    {
        return m_full;
    }

    const tstring& get_dir() const
    {
        return m_dir;
    }

    const tstring& get_name() const
    {
        return m_name;
    }

    const tstring& get_ext() const
    {
        return m_ext;
    }

private:
    bool m_has_init;

    tstring m_full;
    tstring m_dir;
    tstring m_name;
    tstring m_ext;
};


