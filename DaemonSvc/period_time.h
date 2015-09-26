#pragma once


struct PeriodTime
{
    enum PERIOD_TYPE
    {
        DAILY,
        WEEKLY,
        MONTHLY,
    };

    PERIOD_TYPE type;
    unsigned short dayofmonth;//1-31
    unsigned short dayofweek;//Sunday is 0, 0-6
    unsigned short hour;//0-23
    unsigned short minute;//0-59
    //unsigned short second;//0-59

    PeriodTime()
        : dayofmonth(0)
        , dayofweek(0)
        , hour(0)
        , minute(0)
        //, second(0)
    {
    }
};


