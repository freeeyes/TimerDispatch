// WindowsTimer_Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "TimerThread.h"
#include "TimerCommon.h"

int Do_Timer_Event(void* pArg)
{
    int* pData = (int*)pArg;
    TS_TIMER::CTime_Value obj_Time_Value = TS_TIMER::GetTimeofDay();
    printf_s("[Do_Timer_Event]<%s>, Arg=%d.\n", obj_Time_Value.Get_string().c_str(), *pData);
    Sleep(500);

    return 0;
}

int main()
{
    TS_TIMER::CTimerThread objTimerThread;

    TS_TIMER::CTime_Value ttbegin;

    int nID = 1;

    objTimerThread.Init();

    objTimerThread.Add_Timer(1, 10000, ttbegin, Do_Timer_Event, (void*)&nID);

    //test1.Set_Timer_Param(1, 10000);

    //objTimerThread.Add_Timer(reinterpret_cast<TS_TIMER::ITimerInfo*>(&test1));

    objTimerThread.Run();

    /*
    TS_TIMER::CTime_Value obj_Time_Value = TS_TIMER::GetTimeofDay();

    printf_s("[obj_Time_Value]obj_Time_Value=<%s>.\n", obj_Time_Value.Get_string().c_str());

    time_t ttSecond                  = 1;
    TS_TIMER::suseconds_t ttuSeconds = 0;
    TS_TIMER::CTime_Value obj_Time_1 = obj_Time_Value + TS_TIMER::CTime_Value(ttSecond, ttuSeconds);

    printf_s("[obj_Time_Value]obj_Time_1=<%s>.\n", obj_Time_1.Get_string().c_str());

    TS_TIMER::CTime_Value obj_Time_2 = obj_Time_1 - obj_Time_Value;

    printf_s("[obj_Time_Value]obj_Time_2.Get_milliseconds=<%d>.\n", obj_Time_2.Get_milliseconds());
    */

    getchar();
    return 0;
}

