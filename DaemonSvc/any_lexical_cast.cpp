#include <boost/algorithm/string.hpp>
#include "any_lexical_cast.h"


template<>
bool any_lexical_cast(const std::string& src, const bool& fail_value)
{
    std::string src_lower = boost::algorithm::to_lower_copy(src);
    boost::algorithm::trim(src_lower);
    if (src_lower == "true")
    {
        return true;
    }
    else if (src_lower == "false")
    {
        return false;
    }
    else
    {
        return fail_value;
    }
}

template<>
bool any_lexical_cast(const std::wstring& src, const bool& fail_value)
{
    std::wstring src_lower = boost::algorithm::to_lower_copy(src);
    boost::algorithm::trim(src_lower);
    if (src_lower == L"true")
    {
        return true;
    }
    else if (src_lower == L"false")
    {
        return false;
    }
    else
    {
        return fail_value;
    }
}

template<>
std::string any_lexical_cast(const bool& src, const std::string& fail_value)
{
    std::string s;

    if (src)
    {
        s = "true";
    }
    else
    {
        s = "false";
    }

    return s;
}

template<>
std::wstring any_lexical_cast(const bool& src, const std::wstring& fail_value)
{
    std::wstring s;

    if (src)
    {
        s = L"true";
    }
    else
    {
        s = L"false";
    }

    return s;
}


