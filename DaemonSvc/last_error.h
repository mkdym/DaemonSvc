#pragma once
#include <Windows.h>
#include "tdef.h"


//do not use any other user-defined function or class
//this is a base class
class CLastError
{
public:
    CLastError()
        : m_code(GetLastError())
    {
        translate();
    }

    CLastError(const DWORD code)
        : m_code(code)
    {
        translate();
    }

    ~CLastError()
    {
    }

public:
    const DWORD code() const
    {
        return m_code;
    }

    const tstring& str() const
    {
        return m_str;
    }

private:
    void translate();

private:
    const DWORD m_code;
    tstring m_str;
};


void print_last_err(const CLastError& e, const tchar* prefix, ...);


