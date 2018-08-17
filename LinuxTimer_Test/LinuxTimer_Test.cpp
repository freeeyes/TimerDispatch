#include <unistd.h>
#include "TimerThread.h"

void Do_Timer_Event(int nTimerID, TS_TIMER::CTime_Value& tvNow, void* pArg, TS_TIMER::EM_Timer_State& emState)
{
    int* pData = (int*)pArg;
    TS_TIMER::CTime_Value obj_Time_Value = TS_TIMER::GetTimeofDay();
    printf("[Do_Timer_Event](%d)<%s>, Arg=%d.\n", nTimerID, obj_Time_Value.Get_string().c_str(), *pData);
	usleep(500*1000);
}

int main()
{
    TS_TIMER::CTimerThread objTimerThread;

    TS_TIMER::CTime_Value ttbegin;

    int nID = 1001;

    objTimerThread.Init();

    objTimerThread.Add_Timer(1, 1000, &ttbegin, Do_Timer_Event, (void*)&nID);

    objTimerThread.Run();
	
    sleep(10);
	//getchar();
    return 0;
}

