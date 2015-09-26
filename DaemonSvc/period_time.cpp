#pragma once
#include <boost/algorithm/string.hpp>
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
        ErrorLogA("can not cast string[%s] to PERIOD_TYPE", s_lower.c_str());
        return UNKNOWN;
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
        ErrorLogA("unknown PERIOD_TYPE: %d", type);
        s = "unknown";
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
        if (log)
        {
            ErrorLogA("type UNKNOWN");
        }
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
        format_ret = sprintf_s(buf, buf_size, "%s period, hour[%d], minute[%d], deviation_minutes[%d]",
            str_type.c_str(), hour, minute, deviation_minutes);
        break;

    case WEEKLY:
        format_ret = sprintf_s(buf, buf_size, "%s period, dayofweek[%d], hour[%d], minute[%d], deviation_minutes[%d]",
            str_type.c_str(), dayofweek, hour, minute, deviation_minutes);
        break;

    case MONTHLY:
        format_ret = sprintf_s(buf, buf_size, "%s period, dayofmonth[%d], hour[%d], minute[%d], deviation_minutes[%d]",
            str_type.c_str(), dayofmonth, hour, minute, deviation_minutes);
        break;

    default:
        break;
    }

    if (-1 == format_ret)
    {
        ErrorLogA("format error, or type unknown");
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
            ErrorLogA("invalid minute: %d", minute);
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
            ErrorLogA("invalid hour: %d", hour);
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
            ErrorLogA("invalid dayofweek: %d", dayofweek);
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
            ErrorLogA("invalid dayofmonth: %d", dayofmonth);
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
        break;
    }

    if (log)
    {
        ErrorLogA("%s deviation_minutes[%d] is too large",
            cast_period_type_to_string(type), deviation_minutes);
    }

    return bReturn;
}


