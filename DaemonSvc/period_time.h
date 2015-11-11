#pragma once
#include <string>


struct PeriodTime
{
    enum PERIOD_TYPE
    {
        DAILY = 0,
        WEEKLY,
        MONTHLY,
    };

    static PERIOD_TYPE cast_period_type_from_string(const std::string& s);
    static std::string cast_period_type_to_string(const PERIOD_TYPE& type);

    PERIOD_TYPE type;
    unsigned short dayofmonth;//1-31
    unsigned short dayofweek;//Sunday is 0, 0-6
    unsigned short hour;//0-23
    unsigned short minute;//0-59
    //unsigned short second;//0-59

    unsigned long deviation_minutes;

    PeriodTime()
        : dayofmonth(0)
        , dayofweek(0)
        , hour(0)
        , minute(0)
        //, second(0)
        , deviation_minutes(0)
    {
    }

    //test if valid
    bool valid(const bool log = false) const;

    //build string for output
    std::string str() const;

private:
    bool valid_minute(const bool log) const;
    bool valid_hour(const bool log) const;
    bool valid_dayofweek(const bool log) const;
    bool valid_dayofmonth(const bool log) const;
    bool valid_deviation_minutes(const bool log) const;

    static void throw_period_type_cast_exception(const std::string& s);
    static void throw_period_type_cast_exception(const PERIOD_TYPE& type);
};


