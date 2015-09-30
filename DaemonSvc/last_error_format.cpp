#include "boost_algorithm_string.h"
#include "last_error_format.h"


bool CLastErrorFormat::translate_error_code(const DWORD code, std::string& s)
{
    char* buf = NULL;

    const DWORD count = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        code,
        0,//MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<char *>(&buf),
        0,
        NULL);

    if (0 == count)
    {
        const DWORD e = GetLastError();

        const size_t error_msg_size = 100;
        char error_msg[error_msg_size] = {0};
        memset(error_msg, 0, sizeof(error_msg));
        sprintf_s(error_msg, "FormatMessageA[%lu] fail, error code: %lu\r\n", code, e);
        s = error_msg;//error_msg is large enough to hold string and a null-terminated ch

        return false;
    }
    else
    {
        //buf is null-terminated, see MSDN
        s = buf;
        LocalFree(buf);

        //buf is always appended by "\r\n"
        boost::algorithm::trim_right_if(s, boost::algorithm::is_any_of("\r\n"));

        return true;
    }
}

bool CLastErrorFormat::translate_error_code(const DWORD code, std::wstring& ws)
{
    wchar_t* buf = NULL;

    const DWORD count = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        code,
        0,//MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<wchar_t *>(&buf),
        0,
        NULL);

    if (0 == count)
    {
        const DWORD e = GetLastError();

        const size_t error_msg_size = 100;
        wchar_t error_msg[error_msg_size] = {0};
        memset(error_msg, 0, sizeof(error_msg));
        swprintf_s(error_msg, L"FormatMessageW[%lu] fail, error code: %lu\r\n", code, e);
        ws = error_msg;//error_msg is large enough to hold error msg and a null-terminated ch

        return false;
    }
    else
    {
        //buf is null-terminated, see MSDN
        ws = buf;
        LocalFree(buf);

        //buf is always appended by "\r\n"
        boost::algorithm::trim_right_if(ws, boost::algorithm::is_any_of(L"\r\n"));

        return true;
    }
}



