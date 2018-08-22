// WindowsTimer_Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "TimerThread.h"
#include "TimerCommon.h"

void Do_Timer_Event(int nTimerID, TS_TIMER::CTime_Value& tvNow, void* pArg, TS_TIMER::EM_Timer_State& emState)
{
    int* pData = (int*)pArg;
    printf_s("[Do_Timer_Event](%d)<%s>, Arg=%d.\n", nTimerID, tvNow.Get_string().c_str(), *pData);
    Sleep(500);
}

void Timeout_Error_Callback(int nErrorTimerID, int nTimerID, vector<TS_TIMER::CTime_Value> vecTimeout, void* pArg)
{
    int* pData = (int*)pArg;
    int nCount = (int)vecTimeout.size();
    printf_s("[Timeout_Error_Callback](nErrorTimerID=%d,nTimerID=%d)<%d>, Arg=%d.\n", nErrorTimerID, nTimerID, nCount, *pData);
}

int main()
{
    TS_TIMER::CTimerThread objTimerThread;

    TS_TIMER::CTime_Value ttbegin = TS_TIMER::GetTimeofDay() - TS_TIMER::CTime_Value(5, 0);

    int nID = 1001;

    objTimerThread.Init();

    objTimerThread.Add_Timer(1, 1000, NULL, Do_Timer_Event, (void*)&nID, Timeout_Error_Callback);

    objTimerThread.Run();

    Sleep(2800);

    objTimerThread.Add_Timer(2, 1000, NULL, Do_Timer_Event, (void*)&nID, Timeout_Error_Callback);

    Sleep(10000);

    getchar();
    return 0;
}

