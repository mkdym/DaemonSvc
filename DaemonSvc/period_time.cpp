#pragma once
#include <exception>
#include "boost_algorithm_string.h"
#include "logger.h"
#include "period_time.h"



PeriodTime::PERIOD_TYPE PeriodTime::cast_period_type_from_string(const std::string& s)
{
    std::string s_lower = boost::algorithm::to_lower_copy(s);
    boost::algorithm::trim(s_lower);

    if (s_lower == "daily")
    {
        return DAILY;
    }
    else if (s_lower == "weekly")
    {
        return WEEKLY;
    }
    else if (s_lower == "monthly")
    {
        return MONTHLY;
    }
    else
    {
        throw_period_type_cast_exception(s_lower);
        return MONTHLY;//will never execute
    }
}

std::string PeriodTime::cast_period_type_to_string(const PERIOD_TYPE& type)
{
    std::string s;
    switch (type)
    {
    case DAILY:
        s = "daily";
        break;

    case WEEKLY:
        s = "weekly";
        break;

    case MONTHLY:
        s = "monthly";
        break;

    default:
        throw_period_type_cast_exception(type);
        break;
    }
    return s;
}


bool PeriodTime::valid(const bool log /*= false*/) const
{
    bool bReturn = false;

    switch (type)
    {
    case DAILY:
        bReturn = valid_hour(log)
            && valid_minute(log)
            && valid_deviation_minutes(log);
        break;

    case WEEKLY:
        bReturn = valid_dayofweek(log)
            && valid_hour(log)
            && valid_minute(log)
            && valid_deviation_minutes(log);
        break;

    case MONTHLY:
        bReturn = valid_dayofmonth(log)
            && valid_hour(log)
            && valid_minute(log)
            && valid_deviation_minutes(log);
        break;

    default:
        throw_period_type_cast_exception(type);
        break;
    }

    return bReturn;
}

std::string PeriodTime::str() const
{
    std::string s;

    const int buf_size = 500;
    char buf[buf_size] = {0};
    int format_ret = -1;
    std::string str_type = cast_period_type_to_string(type);
    switch (type)
    {
    case DAILY:
        format_ret = sprintf_s(buf, buf_size, "%s period, hour[%lu], minute[%lu], deviation_minutes[%lu]",
            str_type.c_str(), hour, minute, deviation_minutes);
        break;

    case WEEKLY:
        format_ret = sprintf_s(buf, buf_size, "%s period, dayofweek[%lu], hour[%lu], minute[%lu], deviation_minutes[%lu]",
            str_type.c_str(), dayofweek, hour, minute, deviation_minutes);
        break;

    case MONTHLY:
        format_ret = sprintf_s(buf, buf_size, "%s period, dayofmonth[%lu], hour[%lu], minute[%lu], deviation_minutes[%lu]",
            str_type.c_str(), dayofmonth, hour, minute, deviation_minutes);
        break;

    default:
        throw_period_type_cast_exception(type);
        break;
    }

    if (-1 == format_ret)
    {
        ErrorLog("format error");
    }
    else
    {
        s.append(buf, format_ret);
    }

    return s;
}

bool PeriodTime::valid_minute(const bool log) const
{
    if (minute <= 59)
    {
        return true;
    }
    else
    {
        if (log)
        {
            ErrorLog("invalid minute: %lu", minute);
        }
        return false;
    }
}

bool PeriodTime::valid_hour(const bool log) const
{
    if (hour <= 23)
    {
        return true;
    }
    else
    {
        if (log)
        {
            ErrorLog("invalid hour: %lu", hour);
        }
        return false;
    }
}

bool PeriodTime::valid_dayofweek(const bool log) const
{
    if (dayofweek <= 6)
    {
        return true;
    }
    else
    {
        if (log)
        {
            ErrorLog("invalid dayofweek: %lu", dayofweek);
        }
        return false;
    }
}

bool PeriodTime::valid_dayofmonth(const bool log) const
{
    if (dayofmonth <= 31 && dayofmonth >= 1)
    {
        return true;
    }
    else
    {
        if (log)
        {
            ErrorLog("invalid dayofmonth: %lu", dayofmonth);
        }
        return false;
    }
}

bool PeriodTime::valid_deviation_minutes(const bool log) const
{
    bool bReturn = false;

    switch (type)
    {
    case DAILY:
        bReturn = (deviation_minutes < 1 * 24 * 60 / 2);
        break;

    case WEEKLY:
        bReturn = (deviation_minutes < 7 * 24 * 60 / 2);
        break;

    case MONTHLY:
        bReturn = (deviation_minutes < 28 * 24 * 60 / 2);
        break;

    default:
        throw_period_type_cast_exception(type);
        break;
    }

    if (log && !bReturn)
    {
        ErrorLog("%s deviation_minutes[%lu] is too large",
            cast_period_type_to_string(type).c_str(), deviation_minutes);
    }

    return bReturn;
}

void PeriodTime::throw_period_type_cast_exception(const std::string& s)
{
    std::string s_except("unexpected PERIOD_TYPE string[");
    s_except += s + "]";
    ErrorLog(s_except.c_str());
    throw std::runtime_error(s_except.c_str());
}

void PeriodTime::throw_period_type_cast_exception(const PERIOD_TYPE& type)
{
    const int buf_size = 200;
    char buf[buf_size] = {0};
    sprintf_s(buf, 200, "unexpected PERIOD_TYPE number[%d]", type);
    buf[buf_size - 1] = 0;
    ErrorLog(buf);
    throw std::runtime_error(buf);
}


