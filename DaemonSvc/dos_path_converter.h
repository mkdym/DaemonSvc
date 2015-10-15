#pragma once
#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include "tdef.h"


class CDosPathConverter : public boost::noncopyable
{
public:
    CDosPathConverter(void);
    ~CDosPathConverter(void);

public:
    std::wstring to_dos_path(const std::wstring& native_name) const;
    std::wstring to_native_path(const std::wstring& dos_name) const;

    //have disabled wow64 fs redirection inner
    static tstring to_long_path_name(const tstring& short_path_name);

private:
    void scan_all_logical_drives();
    static std::wstring query_symlink_until(const std::wstring& link);
    static std::wstring query_symlink_once(const std::wstring& link);

private:
    typedef std::map<std::wstring, std::wstring> DriveNameMap;
    DriveNameMap m_dos2native;
};
