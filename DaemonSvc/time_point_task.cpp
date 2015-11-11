#include <cassert>
#include <Windows.h>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "exception_catcher.h"
#include "logger.h"
#include "time_point_task.h"



static const boost::posix_time::ptime EPOCH_PTIME(boost::gregorian::date(1970, 1, 1),
                                            boost::posix_time::time_duration(0, 0, 0));



CTimePointTask::CTimePointTask(const TaskFunc& f, const PeriodTime& period)
    : m_started(false)
    , m_last_executed_time(0)
    , m_f(f)
    , m_period(period)
{

}

CTimePointTask::~CTimePointTask(void)
{
    if (m_started)
    {
        stop();
    }
}

bool CTimePointTask::is_started() const
{
    return m_started;
}

bool CTimePointTask::start()
{
    if (m_started)
    {
        return true;
    }
    else
    {
        m_exit_event.reset(CreateEvent(NULL, TRUE, FALSE, NULL));
        if (!m_exit_event.valid())
        {
            ErrorLogLastErr("CreateEvent for notify time point task thread exit fail");
        }
        else
        {
            try
            {
                m_worker_thread = boost::thread(boost::bind(&CTimePointTask::worker_func, this));
                m_started = true;
            }
            catch (boost::thread_resource_error& e)
            {
                ErrorLog("create time point task worker thread fail, error: %s", e.what());
            }
        }

        return m_started;
    }
}

void CTimePointTask::stop()
{
    if (m_started)
    {
        assert(m_exit_event.valid());

        SetEvent(m_exit_event.get_ref());
        if (m_worker_thread.joinable())
        {
            m_worker_thread.join();
        }

        m_started = false;
    }
}

void CTimePointTask::worker_func()
{
    exception_catcher::set_thread_exception_handlers();
    InfoLog("time point task worker thread func begin");

    while (true)
    {
        //计算上一次应执行的时间
        const seconds_t last_should_execute_time = get_last_should_execute_time();
        //看当前时间和上一次应执行时间之间有没有被执行过，即上次执行时间是否在上一次应执行时间和当前时间之间
        if (!has_executed_after(last_should_execute_time))//若没有，则判断是否在误差范围内，即当前时间-上一次应执行时间 是否 <= 误差
        {
            const seconds_t cur_time = get_local_time();
            const seconds_t diff_in_minutes = (cur_time - last_should_execute_time) / 60;
            if (diff_in_minutes <= m_period.deviation_minutes)//若在误差范围内，则执行，并更新上一次执行时间
            {
                if (m_f)
                {
                    try
                    {
                        m_f();
                    }
                    catch (...)
                    {
                        ErrorLog("execute time point task function exception");
                    }
                }

                m_last_executed_time = cur_time;
            }
        }

        //等待时间：等待误差时间的一半，若这一半大于10分钟，则以10分钟计
        unsigned long wait_minutes = m_period.deviation_minutes / 2;
        if (wait_minutes > 10)
        {
            wait_minutes = 10;
        }
        const DWORD wait_seconds = wait_minutes ? wait_minutes * 60 : 30;

        const DWORD wait_result = WaitForSingleObject(m_exit_event.get_ref(), wait_seconds * 1000);
        bool should_break = false;
        switch (wait_result)
        {
        case WAIT_OBJECT_0:
            InfoLog("got exit notify");
            should_break = true;
            break;

        case WAIT_TIMEOUT:
            break;

        default:
            ErrorLogLastErr("WaitForSingleObject fail, return code: %lu", wait_result);
            should_break = true;
            break;
        }

        if (should_break)
        {
            break;
        }
    }

    InfoLog("time point task worker thread func end");
}

CTimePointTask::seconds_t CTimePointTask::get_last_should_execute_time()
{
    boost::posix_time::ptime last_should_execute_time;
    const boost::posix_time::ptime cur_time(boost::posix_time::second_clock::local_time());
    const boost::posix_time::time_duration td(m_period.hour, m_period.minute, 0);

    switch (m_period.type)
    {
    case PeriodTime::DAILY:
        {
            const boost::posix_time::ptime cur_should_execute_time(cur_time.date(), td);
            if (cur_time > cur_should_execute_time)
            {
                last_should_execute_time = cur_should_execute_time;
            }
            else
            {
                last_should_execute_time = cur_should_execute_time - boost::gregorian::days(1);
            }
        }
        break;

    case PeriodTime::WEEKLY:
        {
            const boost::gregorian::days diff_days(cur_time.date().day_of_week() - m_period.dayofweek);
            const boost::posix_time::ptime cur_should_execute_time(cur_time.date() - diff_days, td);
            if (cur_time > cur_should_execute_time)
            {
                last_should_execute_time = cur_should_execute_time;
            }
            else
            {
                last_should_execute_time = cur_should_execute_time - boost::gregorian::days(7);
            }
        }
        break;

    case PeriodTime::MONTHLY:
        {
            const boost::gregorian::days diff_days(cur_time.date().day() - m_period.dayofmonth);
            const boost::posix_time::ptime cur_should_execute_time(cur_time.date() - diff_days, td);
            if (cur_time > cur_should_execute_time)
            {
                last_should_execute_time = cur_should_execute_time;
            }
            else
            {
                last_should_execute_time = cur_should_execute_time - boost::gregorian::months(1);
            }
        }
        break;

    default:
        break;
    }

    assert(!last_should_execute_time.is_not_a_date_time());
    const boost::posix_time::time_duration::sec_type total_seconds = (last_should_execute_time - EPOCH_PTIME).total_seconds();
    return total_seconds;
}

bool CTimePointTask::has_executed_after(const seconds_t& begin_time)
{
    if (0 == m_last_executed_time)
    {
        return false;
    }
    else
    {
        const seconds_t cur_time = get_local_time();
        if (begin_time > cur_time)//时间被往回调了，那么多执行一次也无妨
        {
            return false;
        }
        else
        {
            return ((m_last_executed_time >= begin_time)
                && (m_last_executed_time <= cur_time));
        }
    }
}

CTimePointTask::seconds_t CTimePointTask::get_local_time()
{
    const boost::posix_time::ptime cur_time(boost::posix_time::second_clock::local_time());
    const boost::posix_time::time_duration::sec_type total_seconds = (cur_time - EPOCH_PTIME).total_seconds();
    return total_seconds;
}


