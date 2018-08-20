#include <unistd.h>
#include "TimerThread.h"

void Do_Timer_Event(int nTimerID, TS_TIMER::CTime_Value& tvNow, void* pArg, TS_TIMER::EM_Timer_State& emState)
{
    int* pData = (int*)pArg;
    TS_TIMER::CTime_Value obj_Time_Value = TS_TIMER::GetTimeofDay();
    printf("[Do_Timer_Event](%d)<%s>, Arg=%d.\n", nTimerID, obj_Time_Value.Get_string().c_str(), *pData);
	usleep(500*1000);
}

void Timeout_Error_Callback(int nTimerID, vector<TS_TIMER::CTime_Value> vecTimeout, void* pArg)
{
    int* pData = (int*)pArg;
    int nCount = (int)vecTimeout.size();
    printf("[Timeout_Error_Callback](%d)<%d>, Arg=%d.\n", nTimerID, nCount, *pData);
}

int main()
{
    TS_TIMER::CTimerThread objTimerThread;

    TS_TIMER::CTime_Value ttbegin = TS_TIMER::GetTimeofDay() - TS_TIMER::CTime_Value(5, 0);

    int nID = 1001;

    objTimerThread.Init();

    objTimerThread.Add_Timer(1, 1000, &ttbegin, Do_Timer_Event, (void*)&nID, Timeout_Error_Callback);

    objTimerThread.Run();
	
    sleep(10);
	//getchar();
    return 0;
}

