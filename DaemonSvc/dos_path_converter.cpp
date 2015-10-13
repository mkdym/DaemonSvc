#include <cassert>
#include <limits>
#include <algorithm>
#include <Windows.h>
#include <boost/smart_ptr/scoped_array.hpp>
#include <boost/thread/once.hpp>
#include "boost_algorithm_string.h"
#include "logger.h"
#include "windows_util.h"
#include "dos_path_converter.h"



typedef struct _UNICODE_STRING
{
    unsigned short Length;
    unsigned short MaximumLength;
    wchar_t* Buffer;
} UNICODE_STRING, *PUNICODE_STRING;


typedef struct _OBJECT_ATTRIBUTES
{
    unsigned long Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    unsigned long Attributes;
    void* SecurityDescriptor;
    void* SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;


#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
}



typedef NTSTATUS (WINAPI *fnNtOpenSymbolicLinkObject)(PHANDLE  LinkHandle,
                                             DWORD DesiredAccess,
                                             POBJECT_ATTRIBUTES ObjectAttributes);

typedef NTSTATUS (WINAPI *fnNtQuerySymbolicLinkObject)(HANDLE LinkHandle,
                                              PUNICODE_STRING LinkTarget,
                                              unsigned long* ReturnedLength);

typedef NTSTATUS (WINAPI *fnNtClose)(HANDLE Handle);


static fnNtOpenSymbolicLinkObject g_fnNtOpenSymbolicLinkObject = NULL;
static fnNtQuerySymbolicLinkObject g_fnNtQuerySymbolicLinkObject = NULL;
static fnNtClose g_fnNtClose = NULL;


static void load_query_funcs()
{
    g_fnNtOpenSymbolicLinkObject = reinterpret_cast<fnNtOpenSymbolicLinkObject>
        (WindowsUtil::load_function("ntdll.dll", "NtOpenSymbolicLinkObject"));

    g_fnNtQuerySymbolicLinkObject = reinterpret_cast<fnNtQuerySymbolicLinkObject>
        (WindowsUtil::load_function("ntdll.dll", "NtQuerySymbolicLinkObject"));

    g_fnNtClose = reinterpret_cast<fnNtClose>
        (WindowsUtil::load_function("ntdll.dll", "NtClose"));
}


static boost::once_flag once_;


CDosPathConverter::CDosPathConverter(void)
{
    boost::call_once(once_, load_query_funcs);
    scan_all_logical_drives();
}

CDosPathConverter::~CDosPathConverter(void)
{
}

std::wstring CDosPathConverter::to_dos_path(const std::wstring& native_name) const
{
    std::wstring dos_path;

    for (DriveNameMap::const_iterator iter_name = m_dos2native.begin();
        iter_name != m_dos2native.end();
        ++iter_name)
    {
        if (boost::algorithm::istarts_with(native_name, iter_name->second))
        {
            dos_path = boost::algorithm::ireplace_first_copy(native_name, iter_name->second, iter_name->first);
            break;
        }
    }

    return dos_path;
}

std::wstring CDosPathConverter::to_native_path(const std::wstring& dos_name) const
{
    std::wstring native_path;

    for (DriveNameMap::const_iterator iter_name = m_dos2native.begin();
        iter_name != m_dos2native.end();
        ++iter_name)
    {
        if (boost::algorithm::istarts_with(dos_name, iter_name->first))
        {
            native_path = boost::algorithm::ireplace_first_copy(dos_name, iter_name->first, iter_name->second);
            break;
        }
    }

    return native_path;
}

void CDosPathConverter::scan_all_logical_drives()
{
    const DWORD drives = GetLogicalDrives();
    for (unsigned char index = 0; index != 26; ++index)//26 drives at most
    {
        const DWORD test = 1L << index;
        if (test & drives)
        {
            static const std::wstring DOS_PREFIX(L"\\DosDevices\\");

            const wchar_t drive_letter = index + L'A';
            std::wstring link;
            link += drive_letter;
            link += L':';
            std::wstring target = query_symlink_until(DOS_PREFIX + link);
            if (target != link)
            {
                m_dos2native[link] = target;
                DebugLog(TSTR("link[%s] target[%s]"), link.c_str(), target.c_str());
            }
        }
    }
}

std::wstring CDosPathConverter::query_symlink_until(const std::wstring& link)
{
    std::wstring cur_link = link;
    while (true)
    {
        std::wstring cur_target = query_symlink_once(cur_link);
        if (cur_target.empty())
        {
            break;
        }

        cur_link = cur_target;
    }

    return cur_link;
}

std::wstring CDosPathConverter::query_symlink_once(const std::wstring& link)
{
    assert(!link.empty());
    assert((link.size() + 1) * sizeof(wchar_t) <= (std::numeric_limits<std::wstring::size_type>::max)());

    std::wstring target;

    if (NULL == g_fnNtOpenSymbolicLinkObject
        || NULL == g_fnNtQuerySymbolicLinkObject
        || NULL == g_fnNtClose)
    {
        return target;
    }

    static const DWORD _OBJ_CASE_INSENSITIVE = 0x00000040L;
    static const DWORD _GENERIC_READ = 0x80000000L;
    static const DWORD _STATUS_BUFFER_TOO_SMALL = 0xC0000023L;
    static const DWORD _STATUS_OBJECT_TYPE_MISMATCH = 0xC0000024L;

    do 
    {
        long query_error_code = 0;

        const unsigned short link_str_bytes = static_cast<unsigned short>(link.size()) * sizeof(wchar_t);
        const unsigned short link_buffer_bytes = (static_cast<unsigned short>(link.size()) + 1) * sizeof(wchar_t);

        boost::scoped_array<wchar_t> link_buffer(new wchar_t[link.size() + 1]);
        memcpy_s(link_buffer.get(), link_buffer_bytes, link.c_str(), link_str_bytes);
        link_buffer[link.size()] = L'\0';

        UNICODE_STRING link_in_us = {0};
        link_in_us.Length = link_str_bytes;
        link_in_us.MaximumLength = link_buffer_bytes;
        link_in_us.Buffer = link_buffer.get();

        OBJECT_ATTRIBUTES oa = {0};
        InitializeObjectAttributes(&oa, &link_in_us, _OBJ_CASE_INSENSITIVE, 0, 0);

        HANDLE hLink = NULL;
        query_error_code = g_fnNtOpenSymbolicLinkObject(&hLink, _GENERIC_READ, &oa);

        if (query_error_code < 0)//fail
        {
            if (query_error_code != _STATUS_OBJECT_TYPE_MISMATCH)
            {
                ErrorLog(TSTR("NtOpenSymbolicLinkObject[%s] fail, error code: 0x%x"), link.c_str(), query_error_code);
            }
            break;
        }

        const DWORD MAX_US_PATH = 1024;
        wchar_t target_buffer[MAX_US_PATH] = {0};

        UNICODE_STRING target_in_us = {0};
        target_in_us.Length = 0;
        target_in_us.MaximumLength = 1024 * sizeof(wchar_t);
        target_in_us.Buffer = target_buffer;

        unsigned long ret_len = 0;
        query_error_code = g_fnNtQuerySymbolicLinkObject(hLink, &target_in_us, &ret_len);
        g_fnNtClose(hLink);
        assert(query_error_code != _STATUS_BUFFER_TOO_SMALL && ret_len <= target_in_us.MaximumLength);

        if (query_error_code < 0)//fail
        {
            ErrorLog(TSTR("NtQuerySymbolicLinkObject[%s] fail, error code: 0x%x"), link.c_str(), query_error_code);
            break;
        }

        //target.append(target_in_us.Buffer, ret_len / sizeof(wchar_t));
        target = target_in_us.Buffer;

    } while (false);

    return target;
}

tstring CDosPathConverter::to_long_path_name(const tstring& short_path_name)
{
    assert(!short_path_name.empty());

    tstring long_path_name;

    do 
    {
        const DWORD buf_need_size = GetLongPathName(short_path_name.c_str(), NULL, 0);
        CLastErrorFormat e_1st;
        if (0 == buf_need_size)
        {
            ErrorLogLastErrEx(e_1st, TSTR("GetLongPathName for [%s] to query need size fail"), short_path_name.c_str());
            break;
        }

        boost::scoped_array<tchar> buf(new tchar[buf_need_size]);
        memset(buf.get(), 0, sizeof(tchar) * buf_need_size);

        const DWORD buf_real_size = GetLongPathName(short_path_name.c_str(), buf.get(), buf_need_size);
        CLastErrorFormat e_2nd;
        if (buf_real_size > buf_need_size)
        {
            ErrorLogLastErrEx(e_2nd, TSTR("GetLongPathName for [%s] fail"), short_path_name.c_str());
            break;
        }

        long_path_name.append(buf.get(), buf_real_size);

    } while (false);

    return long_path_name;
}



