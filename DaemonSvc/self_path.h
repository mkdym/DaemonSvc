#pragma once
#include <string>
#include "singleton.h"


//do not use log, because we use this class to init log module
//only support char version, in order to reduce potential errors
//if you need wchar_t string, you can use "ansistr2widestr" to do string conversion
class CSelfPath : public Singleton<CSelfPath>
{
    friend class Singleton<CSelfPath>;

private:
    CSelfPath(void);

public:
    ~CSelfPath(void)
    {
    }

public:
    bool valid() const
    {
        return !(m_full.empty());
    }

public:
    const std::string& get_full() const
    {
        return m_full;
    }

    const std::string& get_dir() const
    {
        return m_dir;
    }

    const std::string& get_name() const
    {
        return m_name;
    }

    const std::string& get_ext() const
    {
        return m_ext;
    }

private:
    std::string m_full;
    std::string m_dir;
    std::string m_name;
    std::string m_ext;
};


