#pragma once
#include <boost/function.hpp>


typedef boost::function<void()> TaskFunc;


enum PERIOD_TYPE
{
    DAILY,
    WEEKLY,
    MONTHLY,
};


struct TaskTime
{
    unsigned char dayofmonth;
    unsigned char dayofweek;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
};



