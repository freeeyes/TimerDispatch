#include "Time_Value.h"
namespace TS_TIMER
{
    CTime_Value::CTime_Value(time_t tv_sec, suseconds_t tv_usec) : m_tv_sec(0), m_tv_usec(0)
    {
        Set_time(tv_sec, tv_usec);
    }

    void CTime_Value::Set_time(time_t tv_sec, suseconds_t tv_usec)
    {
        m_tv_sec = tv_sec;
        m_tv_usec = tv_usec;
    }

    time_t CTime_Value::Get_sec() const
    {
        return m_tv_sec;
    }

    suseconds_t CTime_Value::Get_usec() const
    {
        return m_tv_usec;
    }

    void CTime_Value::normalize()
    {
        //将时间规格化成一个标准的时间类型
        if (m_tv_usec >= ONE_SECOND_IN_USECS)
        {
            do
            {
                ++m_tv_sec;
                m_tv_usec -= ONE_SECOND_IN_USECS;
            }
            while (m_tv_usec >= ONE_SECOND_IN_USECS);
        }
        else if (m_tv_usec <= -ONE_SECOND_IN_USECS)
        {
            do
            {
                --m_tv_sec;
                m_tv_usec += ONE_SECOND_IN_USECS;
            }
            while (m_tv_usec <= -ONE_SECOND_IN_USECS);
        }

        if (m_tv_sec >= 1 && m_tv_usec < 0)
        {
            --m_tv_sec;
            m_tv_usec += ONE_SECOND_IN_USECS;
        }
    }

    std::string CTime_Value::Get_string()
    {
        char szData[100] = { '\0' };

        if (true == IsZero())
        {
#ifdef WIN32
            sprintf_s(szData, 50, "1970-01-01 00:00:00");
#else
            sprintf(szData, "1970-01-01 00:00:00");
#endif
        }
        else
        {
            struct tm tmDate;

#ifdef WIN32
            localtime_s(&tmDate, &m_tv_sec);

            sprintf_s(szData, 50, "%04d-%02d-%02d %02d:%02d:%02d %03d", tmDate.tm_year + 1900,
                      tmDate.tm_mon + 1,
                      tmDate.tm_mday,
                      tmDate.tm_hour,
                      tmDate.tm_min,
                      tmDate.tm_sec,
                      (int)(m_tv_usec / 1000));

#else
            localtime_r(&m_tv_sec, &tmDate);

            sprintf(szData, "%04d-%02d-%02d %02d:%02d:%02d %03d", tmDate.tm_year + 1900,
                    tmDate.tm_mon + 1,
                    tmDate.tm_mday,
                    tmDate.tm_hour,
                    tmDate.tm_min,
                    tmDate.tm_sec,
                    (int)(m_tv_usec / 1000));

#endif
        }

        return (string)szData;
    }

    long CTime_Value::Get_milliseconds()
    {
        long lmilliseconds = (long)m_tv_sec * 1000 + m_tv_usec / 1000;
        return lmilliseconds;
    }

    bool CTime_Value::IsZero()
    {
        if (m_tv_sec == 0 && m_tv_usec == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

}
