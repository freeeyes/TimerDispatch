#include <unistd.h>
#include "TimerThread.h"

int Do_Timer_Event(void* pArg)
{
    int* pData = (int*)pArg;
    TS_TIMER::CTime_Value obj_Time_Value = TS_TIMER::GetTimeofDay();
    printf("[Do_Timer_Event]<%s>, Arg=%d.\n", obj_Time_Value.Get_string().c_str(), *pData);
	usleep(500*1000);
	return 0;
}

int main()
{
    TS_TIMER::CTimerThread objTimerThread;

    TS_TIMER::CTime_Value ttbegin;

    int nID = 1;

    objTimerThread.Init();

    objTimerThread.Add_Timer(1, 1000, ttbegin, Do_Timer_Event, (void*)&nID);

    objTimerThread.Run();
	
    sleep(10);
	//getchar();
    return 0;
}

