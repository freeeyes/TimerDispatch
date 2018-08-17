#ifndef _TIMER_COMMON_H
#define _TIMER_COMMON_H

#include "Time_Value.h"
#ifdef WIN32
#include "windows.h"
#else
#include<sys/time.h>
#endif

//timer的一些公用API函数
//add by freeyes

namespace TS_TIMER
{
    inline CTime_Value GetTimeofDay()
    {
        CTime_Value obj_Time_Value;

#ifdef WIN32

        FILETIME   tfile;
        ::GetSystemTimeAsFileTime(&tfile);

        ULARGE_INTEGER _100ns;
        _100ns.LowPart = tfile.dwLowDateTime;
        _100ns.HighPart = tfile.dwHighDateTime;

        ULARGE_INTEGER epoch; // UNIX epoch (1970-01-01 00:00:00) expressed in Windows NT FILETIME
        epoch.LowPart  = 0xD53E8000;
        epoch.HighPart = 0x019DB1DE;

        _100ns.QuadPart -= epoch.QuadPart;

        // Convert 100ns units to seconds;
        time_t tv_sec = (time_t)(_100ns.QuadPart / (10000 * 1000));
        // Convert remainder to microseconds;
        suseconds_t tv_usec = (suseconds_t)((_100ns.QuadPart % (10000 * 1000)) / 10);

        obj_Time_Value.Set_time(tv_sec, tv_usec);
#else
        struct  timeval    tvNow;
        struct  timezone   tz;
        gettimeofday(&tvNow, &tz);
        time_t tv_sec = (time_t)tvNow.tv_sec;
        suseconds_t tv_usec = (suseconds_t)tvNow.tv_usec;

        obj_Time_Value.Set_time(tv_sec, tv_usec);
#endif
        return obj_Time_Value;
    }
};

#endif