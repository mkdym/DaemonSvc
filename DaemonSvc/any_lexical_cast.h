#pragma once
#include <string>
#include <boost/lexical_cast.hpp>


template<typename Target, typename Source>
Target any_lexical_cast(const Source& src, const Target& fail_value)
{
    Target value = fail_value;
    try
    {
        value = boost::lexical_cast<Target>(src);
    }
    catch (boost::bad_lexical_cast&)
    {
        value = fail_value;
    }
    return value;
}

template<>
bool any_lexical_cast<bool, std::string>(const std::string& src, const bool& fail_value);

template<>
bool any_lexical_cast<bool, std::wstring>(const std::wstring& src, const bool& fail_value);

template<>
std::string any_lexical_cast<std::string, bool>(const bool& src, const std::string&);

template<>
std::wstring any_lexical_cast<std::wstring, bool>(const bool& src, const std::wstring&);


template<typename CharType, typename Source>
std::basic_string<CharType> string_lexical_cast(const Source& src)
{
    static const std::basic_string<CharType> empty_str;
    return any_lexical_cast<std::basic_string<CharType>, Source>(src, empty_str);
}


