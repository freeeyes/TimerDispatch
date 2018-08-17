// WindowsTimer_Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "TimerThread.h"
#include "TimerCommon.h"

void Do_Timer_Event(int nTimerID, void* pArg, TS_TIMER::EM_Timer_State& emState)
{
    int* pData = (int*)pArg;
    TS_TIMER::CTime_Value obj_Time_Value = TS_TIMER::GetTimeofDay();
    printf("sec=%d, usec=%d.\n", obj_Time_Value.Get_sec(), obj_Time_Value.Get_usec()/1000);
    //printf_s("[Do_Timer_Event](%d)<%d>, Arg=%d.\n", nTimerID, obj_Time_Value.Get_usec(), *pData);
    //Sleep(50);
}


uint64_t update()
{
    uint64_t _ts;
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER epoch; // UNIX epoch (1970-01-01 00:00:00) expressed in Windows NT FILETIME
    epoch.LowPart = 0xD53E8000;
    epoch.HighPart = 0x019DB1DE;

    ULARGE_INTEGER ts;
    ts.LowPart = ft.dwLowDateTime;
    ts.HighPart = ft.dwHighDateTime;
    ts.QuadPart -= epoch.QuadPart;
    _ts = ts.QuadPart / (10 * 1000);
    return _ts;
}

int main()
{
    TS_TIMER::CTimerThread objTimerThread;

    TS_TIMER::CTime_Value ttbegin;

    int nID = 1001;

    objTimerThread.Init();

    objTimerThread.Add_Timer(1, 100, &ttbegin, Do_Timer_Event, (void*)&nID);

    objTimerThread.Run();

    Sleep(1000);

    return 0;
}

