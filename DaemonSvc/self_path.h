#pragma once
#include "singleton.h"
#include "tdef.h"


//do not use log, because we use this class to init log module
class CSelfPath : public Singleton<CSelfPath>
{
    friend class Singleton<CSelfPath>;

private:
    CSelfPath(void)
        : m_has_init(false)
    {
    }

public:
    ~CSelfPath(void)
    {
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


